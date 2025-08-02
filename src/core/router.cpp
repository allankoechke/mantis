#include "../../include/mantis/core/router.h"

#include "../../include/mantis/utils/utils.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/core/tables/sys_tables.h"
#include "../../include/mantis/core/fileunit.h"

#include <format>
#include <nlohmann/json.hpp>

#include "mantis/core/settings.h"
using json = nlohmann::json;

#include<cmrc/cmrc.hpp>

// Declare a mantis namespace for the embedded FS
CMRC_DECLARE(mantis);

#define __file__ "core/router.cpp"

mantis::Router::Router()
{
    // Create admin table object, we'll use it to get JSON rep for use in
    // the TableUnit construction. Similar to what we do when creating routes.
    AdminTable admin;
    admin.name = "__admins";
    admin.id = TableUnit::generateTableId("__admins");
    auto admin_obj = admin.to_json();

    adminTableFields = admin_obj.value("fields", json::array());
    m_adminTable = std::make_shared<TableUnit>(admin_obj);
    m_tableRoutes = std::make_shared<SysTablesUnit>("__tables",
                                                    TableUnit::generateTableId("__tables"), "base");

    // Override the route display name to easier names. This means that,
    // instead of `<root>/__admins` -> <root>/admins
    // instead of `<root>/__tables` -> <root>/tables
    m_adminTable->setRouteDisplayName("admins");
    m_tableRoutes->setRouteDisplayName("tables");
}

bool mantis::Router::initialize()
{
    if (!generateTableCrudApis())
        return false;

    // Add Admin Route as the last, should override
    // any existing route
    if (!generateAdminCrudApis())
        return false;

    if (!generateFileServingApi())
        return false;

    // If all was completed with no issues,
    // just return OK!
    return true;
}

bool mantis::Router::listen() const
{
    try
    {
        // Quick hack to mute `This method can be made static` warning on CLion
        [[maybe_unused]] auto s = m_routes.size();
        return MantisApp::instance().http().listen(MantisApp::instance().host(), MantisApp::instance().port());
    }
    catch (const std::exception& e)
    {
        Log::critical("Failed to start server: {}", e.what());
    }
    catch (...)
    {
        Log::critical("Failed to start server: Unknown Error");
    }

    return false;
}

void mantis::Router::close()
{
    MantisApp::instance().http().close();
    m_routes.clear();
}

json mantis::Router::addRoute(const std::string& table)
{
    TRACE_CLASS_METHOD()

    if (trim(table).empty())
    {
        json res;
        res["success"] = false;
        res["error"] = "Table name can't be empty";
        return res;
    }

    try
    {
        const auto sql = MantisApp::instance().db().session();

        soci::row row;
        const std::string query = "SELECT id, name, type, schema, has_api FROM __tables WHERE name = :name";
        *sql << query, soci::use(table), soci::into(row);

        if (!sql->got_data())
        {
            json res;
            res["success"] = false;
            res["error"] = "No table found with the name " + table;
            return res;
        }

        const auto id = row.get<std::string>("id");
        const auto name = row.get<std::string>("name");
        const auto type = row.get<std::string>("type");
        const auto hasApi = row.get<bool>("has_api");

        // If `hasApi` is set, schema is valid, then, add API endpoints
        if (const auto schema = row.get<json>("schema"); (hasApi && !schema.empty()))
        {
            // We need to persist this instance, else it'll be cleaned up causing a crash
            const auto tableUnit = std::make_shared<TableUnit>(schema);
            tableUnit->setTableName(name);
            tableUnit->setTableId(id);

            if (!tableUnit->setupRoutes())
                return false;

            m_routes.push_back(tableUnit);
        }
    }
    catch (const std::exception& e)
    {
        json res;
        res["success"] = false;
        res["error"] = e.what();
        return res;
    }

    json res;
    res["success"] = true;
    res["error"] = "";
    return res;
}

json mantis::Router::updateRoute(const json& table_data)
{
    TRACE_CLASS_METHOD()

    json res;
    res["success"] = false;
    res["error"] = "";

    if (table_data.is_null() || table_data.empty())
    {
        res["error"] = "Table data can't be empty!";
        return res;
    }

    if (!table_data.contains("new_name") || table_data["new_name"].is_null() || table_data["new_name"].empty())
    {
        res["error"] = "Table new name can't be null/empty!";
        return res;
    }

    if (!table_data.contains("old_name") || table_data["old_name"].is_null() || table_data["old_name"].empty())
    {
        res["error"] = "Table old name can't be null/empty!";
        return res;
    }

    if (!table_data.contains("old_type") || table_data["old_type"].is_null() || table_data["old_type"].empty())
    {
        res["error"] = "Table type is required!";
        return res;
    }

    const auto table_name = table_data.at("new_name").get<std::string>();
    const auto table_old_name = table_data.at("old_name").get<std::string>();
    const auto table_type = table_data.at("old_type").get<std::string>();

    // Let's find and remove existing object
    const auto it = std::ranges::find_if(m_routes, [&](const auto& route)
    {
        return route->tableName() == table_old_name;
    });

    if (it == m_routes.end())
    {
        res["error"] = "TableUnit for " + table_old_name + " not found!";
        return res;
    }

    // Also, check if we have defined some routes for this one ...
    const auto basePath = "/api/v1/" + table_old_name;
    MantisApp::instance().http().routeRegistry().remove("GET", basePath);
    MantisApp::instance().http().routeRegistry().remove("GET", basePath + "/:id");

    if (table_type != "view")
    {
        MantisApp::instance().http().routeRegistry().remove("POST", basePath);
        MantisApp::instance().http().routeRegistry().remove("PATCH", basePath + "/:id");
        MantisApp::instance().http().routeRegistry().remove("DELETE", basePath + "/:id");
    }

    if (table_type == "auth")
    {
        MantisApp::instance().http().routeRegistry().remove("POST", basePath + "/auth-with-password");
    }

    // Remove tableUnit instance for the instance
    m_routes.erase(it);

    return addRoute(table_name);
}

mantis::json mantis::Router::updateRouteCache(const json& table_data)
{
    TRACE_CLASS_METHOD()

    json res;
    res["success"] = false;
    res["error"] = "";

    // Get table name
    const auto table_name = table_data.at("name").get<std::string>();

    // Let's find and remove existing object
    const auto it = std::ranges::find_if(m_routes, [&](const auto& route)
    {
        return route->tableName() == table_name;
    });

    if (it == m_routes.end())
    {
        res["error"] = "TableUnit for " + table_name + " not found!";
        return res;
    }

    // Update cached data & rules using the schema
    (*it)->fromJson(table_data);

    res["success"] = true;
    return res;
}

json mantis::Router::removeRoute(const json& table_data)
{
    TRACE_CLASS_METHOD()

    json res;
    res["success"] = false;
    res["error"] = "";

    if (table_data.is_null() || table_data.empty())
    {
        res["error"] = "Table data can't be empty!";
        return res;
    }

    if (!table_data.contains("name") || table_data["name"].is_null() || table_data["name"].empty())
    {
        res["error"] = "Table name can't be null/empty!";
        return res;
    }

    if (!table_data.contains("type") || table_data["type"].is_null() || table_data["type"].empty())
    {
        res["error"] = "Table type is required!";
        return res;
    }

    const auto table_name = table_data.at("name").get<std::string>();
    const auto table_type = table_data.at("type").get<std::string>();

    // Let's find and remove existing object
    const auto it = std::ranges::find_if(m_routes, [&](const auto& route)
    {
        return route->tableName() == table_name;
    });

    if (it == m_routes.end())
    {
        res["error"] = "TableUnit not found!";
        return res;
    }

    // Also, check if we have defined some routes for this one ...

    const auto basePath = "/api/v1/" + table_name;
    MantisApp::instance().http().routeRegistry().remove("GET", basePath);
    MantisApp::instance().http().routeRegistry().remove("GET", basePath + "/:id");

    if (table_type != "view")
    {
        MantisApp::instance().http().routeRegistry().remove("POST", basePath);
        MantisApp::instance().http().routeRegistry().remove("PATCH", basePath + "/:id");
        MantisApp::instance().http().routeRegistry().remove("DELETE", basePath + "/:id");
    }

    if (table_type == "auth")
    {
        MantisApp::instance().http().routeRegistry().remove("POST", basePath + "/auth-with-password");
    }

    // Remove tableUnit instance for the instance
    m_routes.erase(it);

    res["success"] = true;
    return res;
}

bool mantis::Router::generateFileServingApi() const
{
    TRACE_CLASS_METHOD()

    try
    {
        MantisApp::instance().http().Get(
            "/api/files/:table/:filename",
            [this](const Request& req, Response& res, [[maybe_unused]] Context& ctx)
            {
                const auto table_name = req.path_params.at("table");
                const auto file_name = req.path_params.at("filename");

                if (table_name.empty() || file_name.empty())
                {
                    json response;
                    response["error"] = "Table name and file name are required!";
                    response["status"] = "400";
                    response["data"] = json::object();

                    res.status = 400;
                    res.set_content(response, "application/json");
                    return;
                }

                const auto fileMgr = MantisApp::instance().files();
                if (const auto path_opt = fileMgr.getFilePath(table_name, file_name);
                    path_opt.has_value())
                {
                    // Return requested file
                    res.set_file_content(path_opt.value());
                    res.status = 200;
                    return;
                }

                json response;
                response["error"] = "File not found!";
                response["status"] = "404";
                response["data"] = json::object();

                res.status = 404;
                res.set_content(response, "application/json");
            }
        );

        return true;
    }
    catch (std::exception& e)
    {
        Log::critical("Error creating file serving endpoint: {}", e.what());
    }

    return false;
}

bool mantis::Router::generateTableCrudApis()
{
    TRACE_CLASS_METHOD()

    const auto sql = MantisApp::instance().db().session();

    // id created updated schema has_api
    const soci::rowset<soci::row> rs = (sql->prepare << "SELECT id, name, type, schema, has_api FROM __tables");

    for (const auto& row : rs)
    {
        const auto id = row.get<std::string>("id");
        const auto name = row.get<std::string>("name");
        const auto type = row.get<std::string>("type");
        const auto hasApi = row.get<bool>("has_api");

        // If `hasApi` is set, schema is valid, then, add API endpoints
        if (const auto schema = row.get<json>("schema"); (hasApi && !schema.empty()))
        {
            // We need to persist this instance, else it'll be cleaned up causing a crash
            const auto tableUnit = std::make_shared<TableUnit>(schema);
            tableUnit->setTableName(name);
            tableUnit->setTableId(id);

            if (!tableUnit->setupRoutes())
                return false;

            m_routes.push_back(tableUnit);
        }
    }

    return true;
}

bool mantis::Router::generateAdminCrudApis() const
{
    TRACE_CLASS_METHOD()

    try
    {
        // Setup routes for the admin users fetch/create/update/delete
        if (!m_adminTable->setupRoutes())
        {
            Log::critical("Failed to setup admin table routes");
            return false;
        }

        // Setup routes to manage database tables fetch/create/update/delete
        if (!m_tableRoutes->setupRoutes())
        {
            Log::critical("Failed to setup database table routes");
            return false;
        }

        if (!MantisApp::instance().settings().setupRoutes())
        {
            Log::critical("Failed to setup settings routes");
            return false;
        }

        auto getMimeType = [this](const std::string& path) -> std::string
        {
            if (path.ends_with(".js")) return "application/javascript";
            if (path.ends_with(".css")) return "text/css";
            if (path.ends_with(".html")) return "text/html";
            if (path.ends_with(".json")) return "application/json";
            if (path.ends_with(".png")) return "image/png";
            if (path.ends_with(".svg")) return "image/svg+xml";
            return "application/octet-stream";
        };

        MantisApp::instance().http().Get(
            R"(/admin(.*))",
            [getMimeType](const Request& req, Response& res, [[maybe_unused]] Context ctx)
            {
                try
                {
                    const auto fs = cmrc::mantis::get_filesystem();
                    std::string path = req.matches[1];
                    Log::trace("Match 1: {}", path);

                    // Normalize the path
                    if (path.empty() || path == "/")
                    {
                        path = "/qrc/index.html";
                        Log::trace("/qrc/index.html");
                    }
                    else
                    {
                        path = std::format("/qrc{}", path);
                        Log::trace("{}", path);
                    }

                    if (!fs.exists(path))
                    {
                        Log::trace("{} path does not exists", path);

                        // fallback to index.html for React routes
                        path = "/qrc/index.html";
                    }

                    try
                    {
                        res.status = 200;
                        const auto file = fs.open(path);
                        const auto mime = getMimeType(path);
                        Log::trace("File: {}, size: {}, mime: {}", path, file.size(), mime);
                        res.set_content(file.begin(), file.size(), mime);
                    }
                    catch (const std::exception& e)
                    {
                        Log::critical("Error processing request [1]: {}", e.what());
                        res.status = 404;

                        const auto file = fs.open("/qrc/404.html");
                        const auto mime = getMimeType("404.html");
                        res.set_content(file.begin(), file.size(), mime);
                    }
                }
                catch (const std::exception& e)
                {
                    res.status = 500;
                    Log::critical("Error processing request [2]: {}", e.what());
                }
            });

        // Add /public static file serving directory
        if (!MantisApp::instance().http().server().set_mount_point("/", MantisApp::instance().publicDir()))
        {
            Log::critical("Failed to setup mount point directory for '/' at '{}'", MantisApp::instance().publicDir());
            return false;
        }
    }

    catch (const std::exception& e)
    {
        Log::critical("Error creating admin routes: ", e.what());
        return false;
    }

    return true;
}
