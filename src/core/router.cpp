#include "../../include/mantis/core/router.h"

#include "../../include/mantis/utils.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../include/mantis/core/models/tables.h"
#include "../../include/mantis/core/routes/tableroutes.h"


mantis::Router::Router(MantisApp* app)
    : m_app(app),
      m_adminTable(std::make_shared<TableUnit>(m_app, "__admin", "admin", "auth")),
      m_tableRoutes(std::make_shared<TableRoutes>(m_app, "__tables", "tables", "base"))
{
    m_adminTable->setRouteDisplayName("admin");
    m_tableRoutes->setRouteDisplayName("tables");
}

bool mantis::Router::initialize()
{
    if (!generateTableCrudApis())
        return false;


    // if (!attachUserRoutes())
    //     return false;

    // Add Admin Route as the last, should override
    // any existing route
    if (!generateAdminCrudApis())
        return false;

    // If all was completed with no issues,
    // just return OK!
    return true;
}

bool mantis::Router::listen() const
{
    try
    {
        return m_app->http().listen(m_app->host(), m_app->port());
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

void mantis::Router::close() const
{
    m_app->http().close();
}

bool mantis::Router::generateTableCrudApis()
{
    Log::debug("Mantis::ServerMgr::GenerateTableCrudApis");

    auto sql = m_app->db().session();
    soci::transaction tr(*sql);

    // id created updated schema has_api
    soci::row r;
    *sql << "SELECT id,name,type,schema,has_api,created,updated FROM __tables"
        , soci::into(r);

    if (sql->got_data())
    {
        auto id = r.get<std::string>("id");
        auto name = r.get<std::string>("name");
        auto type = r.get<std::string>("type");
        auto schema_str = r.get<std::string>("schema");
        auto has_api = r.get<bool>("has_api");
        auto created = r.get<std::tm>("created");
        auto updated = r.get<std::tm>("updated");

        Log::debug("{} {}", created.tm_year, updated.tm_mon + 1);

        json schema;
        if (auto res = tryParseJsonStr(schema_str); res.has_value())
        {
            schema = std::move(res.value());
        }

        if (!schema.empty())
        {
            Table t;
            t.id = id;
            t.name = name;
            // t.type = type;
            // t.schema = schema["name"];
            // json j;
            // j["id"] = id;
            // j["name"] = name;
            // j["type"] = type;
            // j["schema"] = schema;
            // j["system"] = system;
            // j["fields"] = json::array();
            // j["rules"] = {
            //     {"list", listRule.to_json()},
            //     {"get", getRule.to_json()},
            //     {"add", addRule.to_json()},
            //     {"update", updateRule.to_json()},
            //     {"delete", deleteRule.to_json()}
            // };
            //
            // Log::debug("Table '{}'", table);
            // // For each, create table object
            //
            // if (TableMgr tbMgr{ *this, table, table, "base"}; !tbMgr.SetupRoutes())
            //     return false;

            Log::debug("Table '{}'", name);
            // For each, create table object

            // We need to persist this instance, else it'll be cleaned up causing a crash
            auto tableUnit = std::make_shared<TableUnit>(m_app, name, id, "base");

            if (!tableUnit->setupRoutes())
                return false;

            m_routes.push_back(tableUnit);
        }
    }

    return true;
}

bool mantis::Router::generateAdminCrudApis()
{
    Log::debug("Mantis::ServerMgr::GenerateAdminCrudApis");

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

        // Setup Admin Dashboard
        m_app->http().Get("/admin", [=](const Request& req, Response& res, Context ctx)
        {
            Log::debug("ServerMgr::GenerateAdminCrudApis for {}", req.path);
            ctx.dump();

            // Response Object
            json response;

            // Check for correct admin user ...
            const auto admin = ctx.get<json>("admin");
            if (!admin)
            {
                Log::critical("User ID is required for admin endpoints ...");

                response["status"] = "404";
                response["message"] = "Admin with the provided Id was not found!";

                res.status = 404;
                res.reason = "Not Found";
                res.set_content(response, "application/json");
            }

            else
            {
                Log::critical("User ID set is = '{}'", admin->dump());
            }

            std::cout << req.path << " [END] SUBMIT HANDLER!" << std::endl;
            res.status = 200;
            response["status"] = "OK";
            response["data"] = "Admin crud apis";
            res.set_content(response, "application/json");
            return;
        }, {
            [=](const Request& req, Response& res, Context ctx)-> bool
            {
                std::cout << req.path << " SUBMIT HANDLER MIDDLEWARE!" << std::endl;

                json admin;
                admin["id"] = "123456789";
                admin["name"] = "John Doe";
                admin["password"] = "--";
                admin["email"] = "john.doe@h.com";
                admin["age"] = 23;
                admin["valid"] = true;

                ctx.set<json>("admin", admin);
                ctx.dump();

                return true;
            }
        });
    }

    catch (const std::exception& e)
    {
        Log::critical("Error creating admin routes: ", e.what());
        return false;
    }

    return true;
}

bool mantis::Router::attachUserRoutes() const
{
    // Log::debug("Mantis::ServerMgr::AttachUserRoutes");
    // Just to
    [[maybe_unused]] auto ctx = m_app->http().context();

    return true;
}
