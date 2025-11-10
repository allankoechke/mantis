#include "../../include/mantis/core/router_mgr.h"
#include "../../include/mantis/utils/utils.h"
#include "../../include/mantis/mantisbase.h"
#include "../../include/mantis/core/database_mgr.h"
#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/core/tables/sys_tables.h"
#include "../../include/mantis/core/files_mgr.h"
#include "../../include/mantis/core/private-impl/duktape_custom_types.h"
#include "../../include/mantis/core/settings_mgr.h"

#include <cmrc/cmrc.hpp>

#ifdef MANTIS_ENABLE_SCRIPTING
#include <dukglue/dukglue.h>
#endif

// Declare a mantis namespace for the embedded FS
CMRC_DECLARE(mantis);

#define __file__ "core/router.cpp"

namespace mantis
{
    RouterMgr::RouterMgr()
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

    bool RouterMgr::initialize()
    {
        if (!generateTableCrudApis())
            return false;

        // Add Admin Route as the last, should override
        // any existing route
        if (!generateAdminCrudApis())
            return false;

        if (!generateFileServingApi())
            return false;

        // Add other necessary endpoints
        [[maybe_unused]] auto _ = generateMiscEndpoints();

        // If all was completed with no issues, just return OK!
        return true;
    }

    bool RouterMgr::listen() const
    {
        try
        {
            // Quick hack to mute `This method can be made static` warning on CLion
            [[maybe_unused]] auto s = m_routes.size();
            return MantisBase::instance().http().listen(MantisBase::instance().host(), MantisBase::instance().port());
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

    void RouterMgr::close()
    {
        MantisBase::instance().http().close();
        m_routes.clear();
    }

    json RouterMgr::addRoute(const std::string& table)
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
            const auto sql = MantisBase::instance().db().session();

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

    json RouterMgr::updateRoute(const json& table_data)
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
        MantisBase::instance().http().routeRegistry().remove("GET", basePath);
        MantisBase::instance().http().routeRegistry().remove("GET", basePath + "/:id");

        if (table_type != "view")
        {
            MantisBase::instance().http().routeRegistry().remove("POST", basePath);
            MantisBase::instance().http().routeRegistry().remove("PATCH", basePath + "/:id");
            MantisBase::instance().http().routeRegistry().remove("DELETE", basePath + "/:id");
        }

        if (table_type == "auth")
        {
            MantisBase::instance().http().routeRegistry().remove("POST", basePath + "/auth-with-password");
        }

        // Remove tableUnit instance for the instance
        m_routes.erase(it);

        return addRoute(table_name);
    }

    json RouterMgr::updateRouteCache(const json& table_data)
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

    json RouterMgr::removeRoute(const json& table_data)
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
        MantisBase::instance().http().routeRegistry().remove("GET", basePath);
        MantisBase::instance().http().routeRegistry().remove("GET", basePath + "/:id");

        if (table_type != "view")
        {
            MantisBase::instance().http().routeRegistry().remove("POST", basePath);
            MantisBase::instance().http().routeRegistry().remove("PATCH", basePath + "/:id");
            MantisBase::instance().http().routeRegistry().remove("DELETE", basePath + "/:id");
        }

        if (table_type == "auth")
        {
            MantisBase::instance().http().routeRegistry().remove("POST", basePath + "/auth-with-password");
        }

        // Remove tableUnit instance for the instance
        m_routes.erase(it);

        res["success"] = true;
        return res;
    }

#ifdef MANTIS_ENABLE_SCRIPTING
    void RouterUnit::registerDuktapeMethods()
    {
        const auto& ctx = MantisApp::instance().ctx();
        dukglue_register_method_varargs(ctx, &RouterUnit::bindRoute, "addRoute");
    }
#endif

    bool RouterMgr::generateFileServingApi() const
    {
        TRACE_CLASS_METHOD()

        try
        {
            MantisBase::instance().http().Get(
                "/api/files/:table/:filename",
                [](const MantisRequest& req, MantisResponse& res)
                {
                    const auto table_name = req.getPathParamValue("table");
                    const auto file_name = req.getPathParamValue("filename");

                    if (table_name.empty() || file_name.empty())
                    {
                        json response;
                        response["error"] = "Table name and file name are required!";
                        response["status"] = "400";
                        response["data"] = json::object();

                        res.sendJson(400, response);
                        return;
                    }

                    const auto fileMgr = MantisBase::instance().files();
                    if (const auto path_opt = fileMgr.getFilePath(table_name, file_name);
                        path_opt.has_value())
                    {
                        // Return requested file
                        res.setStatus(200);
                        res.setFileContent(path_opt.value());
                        return;
                    }

                    json response;
                    response["error"] = "File not found!";
                    response["status"] = "404";
                    response["data"] = json::object();

                    res.sendJson(404, response);
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

    bool RouterMgr::generateTableCrudApis()
    {
        TRACE_CLASS_METHOD()

        const auto sql = MantisBase::instance().db().session();

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

    bool RouterMgr::generateMiscEndpoints() const
    {
        TRACE_CLASS_METHOD()
        // Add /health for server health check
        MantisBase::instance().http().Get("/api/v1/health",
                                         [](MantisRequest&, const MantisResponse& res)
                                         {
                                             // Compute system uptime and send to user
                                             const auto& start_time = MantisBase::instance().startTime();
                                             auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
                                                 std::chrono::steady_clock::now() - start_time).count();

                                             json response;
                                             response["status"] = "ok";
                                             response["uptime"] = uptime;
                                             res.sendJson(200, response);
                                         });

        return true;
    }

    bool RouterMgr::generateAdminCrudApis() const
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

            if (!MantisBase::instance().settings().setupRoutes())
            {
                Log::critical("Failed to setup settings routes");
                return false;
            }

            auto getMimeType = [](const std::string& path) -> std::string
            {
                if (path.ends_with(".js")) return "application/javascript";
                if (path.ends_with(".css")) return "text/css";
                if (path.ends_with(".html")) return "text/html";
                if (path.ends_with(".json")) return "application/json";
                if (path.ends_with(".png")) return "image/png";
                if (path.ends_with(".svg")) return "image/svg+xml";
                return "application/octet-stream";
            };

            MantisBase::instance().http().Get(
                R"(/admin(.*))",
                [getMimeType](MantisRequest& req, MantisResponse& res)
                {
                    try
                    {
                        const auto fs = cmrc::mantis::get_filesystem();
                        std::string path = req.matches()[1];

                        // Normalize the path
                        if (path.empty() || path == "/")
                        {
                            path = "/qrc/index.html";
                        }
                        else
                        {
                            path = std::format("/qrc{}", path);
                        }

                        if (!fs.exists(path))
                        {
                            Log::trace("{} path does not exists", path);

                            // fallback to index.html for React routes
                            path = "/qrc/index.html";
                        }

                        try
                        {
                            const auto file = fs.open(path);
                            const auto mime = getMimeType(path);
                            res.setContent(file.begin(), file.size(), mime);
                            res.setStatus(200);
                        }
                        catch (const std::exception& e)
                        {
                            const auto file = fs.open("/qrc/404.html");
                            const auto mime = getMimeType("404.html");

                            res.setContent(file.begin(), file.size(), mime);
                            res.setStatus(404);
                            Log::critical("Error processing /admin response: {}", e.what());
                        }
                    }
                    catch (const std::exception& e)
                    {
                        res.setStatus(500);
                        Log::critical("Error processing /admin request: {}", e.what());
                    }
                });

            // Add /public static file serving directory
            if (!MantisBase::instance().http().server().set_mount_point("/", MantisBase::instance().publicDir()))
            {
                Log::critical("Failed to setup mount point directory for '/' at '{}'",
                              MantisBase::instance().publicDir());
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

#ifdef MANTIS_ENABLE_SCRIPTING
    duk_ret_t RouterUnit::bindRoute(duk_context* ctx)
    {
        // Get method (GET, POST, etc.) from argument 0
        auto method = trim(duk_require_string(ctx, 0));
        std::ranges::transform(method, method.begin(), ::toupper);
        if (method.empty() ||
            !(method == "GET" || method == "POST" || method == "PATCH" || method == "DELETE"))
        {
            duk_error(ctx, DUK_ERR_TYPE_ERROR,
                      "addRoute expects request method of type `GET`, `POST`, `PATCH` or `DELETE` only!");
            return DUK_RET_TYPE_ERROR;
        }

        // Get path from argument 1
        const auto path = trim(duk_require_string(ctx, 1));
        // TODO Catch wildcard paths as well?
        if (path.empty() || path[0] != '/')
        {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "addRoute expects route paths to be valid and start with `/`!");
            return DUK_RET_TYPE_ERROR;
        }

        // Get number of function arguments (everything after path)
        duk_idx_t n = duk_get_top(ctx);
        if (n < 3)
        {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "addRoute requires at least a handler function");
            return DUK_RET_TYPE_ERROR;
        }

        // First function (argument 2) is the handler
        if (!duk_is_callable(ctx, 2))
        {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "Argument 2 must be a callable handler function");
            return DUK_RET_TYPE_ERROR;
        }

        duk_dup(ctx, 2);
        DukValue handler = DukValue::take_from_stack(ctx);

        // Remaining functions (arguments 3+) are middleware
        std::vector<DukValue> middlewares;
        for (duk_idx_t i = 3; i < n; i++)
        {
            if (!duk_is_callable(ctx, i))
            {
                duk_error(ctx, DUK_ERR_TYPE_ERROR, "All arguments after handler must be callable functions");
                return DUK_RET_TYPE_ERROR;
            }

            duk_dup(ctx, i);
            middlewares.push_back(DukValue::take_from_stack(ctx));
        }

        auto& http = MantisApp::instance().http();
        if (method == "GET")
        {
            http.Get(path, [this, ctx, handler, middlewares](
                     MantisRequest& req,
                     MantisResponse& res)
                     {
                         this->executeRoute(ctx, handler, middlewares, req, res);
                     }, {}
            );
        }
        else if (method == "POST")
        {
            http.Post(path, [this, ctx, handler, middlewares](
                      MantisRequest& req,
                      MantisResponse& res)
                      {
                          this->executeRoute(ctx, handler, middlewares, req, res);
                      }, {}
            );
        }
        else if (method == "PATCH")
        {
            http.Patch(path, [this, ctx, handler, middlewares](
                       MantisRequest& req,
                       MantisResponse& res)
                       {
                           this->executeRoute(ctx, handler, middlewares, req, res);
                       }, {}
            );
        }
        else if (method == "DELETE")
        {
            http.Delete(path, [this, ctx, handler, middlewares](
                        MantisRequest& req,
                        MantisResponse& res)
                        {
                            this->executeRoute(ctx, handler, middlewares, req, res);
                        }, {}
            );
        }
        else
        {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "Unsupported HTTP method: %s", method.c_str());
            return DUK_RET_TYPE_ERROR;
        }

        return 0; // No return value
    }

    void RouterUnit::executeRoute(duk_context* ctx, const DukValue& handler, const std::vector<DukValue>& middlewares,
                              MantisRequest& req, MantisResponse& res)
    {
        // Execute middleware functions first
        for (const auto& middleware : middlewares)
        {
            try
            {
                // Call middleware: middleware(req, res)
                // If middleware returns false, stop execution
                const bool ok = dukglue_pcall<bool>(ctx, middleware, &req, &res);

                if (!ok)
                {
                    if (res.getStatus() < 400) res.setStatus(500); // If error code is not explicit
                    return; // Middleware stopped the chain
                }
            }
            catch (const DukException& e)
            {
                json response;
                response["status"] = "ok";
                response["data"] = json::object();
                response["error"] = e.what();

                res.sendJson(500, response);
                Log::critical("Error Executing Middleware: {}", e.what());
                return;
            }
        }

        // Execute the handler function
        try
        {
            dukglue_pcall<void>(ctx, handler, &req, &res);
        }
        catch (const DukException& e)
        {
            json response;
            response["status"] = 500;
            response["data"] = json::object();
            response["error"] = e.what();

            res.sendJson(500, response);
            Log::critical("Error Executing Route {} : {}", req.getPath(), e.what());
        }
    }
#endif
}
