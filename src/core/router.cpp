#include "../../include/mantis/core/router.h"
#include "../../include/mantis/core/database.h"
#include "../../include/mantis/core/models/tables.h"
#include "../../include/mantis/app/app.h"

mantis::Router::Router(MantisApp* app)
    : m_app(app) {}

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
    *sql << "SELECT id,schema,has_api,created,updated FROM __tables"
    , soci::into(r);

    if (sql->got_data())
    {
        auto id = r.get<std::string>("id");
        auto schema = r.get<json>("schema");
        auto has_api = r.get<bool>("has_api");
        auto created = r.get<std::tm>("created");
        auto updated = r.get<std::tm>("updated");

        // if (!schema.empty())
        // {
        //     Table t;
        //     t.id = schema["id"].get<std::string>();
        //     t.name = schema["name"];
        //     t.type = schema["name"];
        //     t.schema = schema["name"];
        //     json j;
        //     j["id"] = id;
        //     j["name"] = name;
        //     j["type"] = type;
        //     j["schema"] = schema;
        //     j["system"] = system;
        //     j["fields"] = json::array();
        //     j["rules"] = {
        //         {"list", listRule.to_json()},
        //         {"get", getRule.to_json()},
        //         {"add", addRule.to_json()},
        //         {"update", updateRule.to_json()},
        //         {"delete", deleteRule.to_json()}
        //     };
        //
        //     Log::debug("Table '{}'", table);
        //     // For each, create table object
        //
        //     if (TableMgr tbMgr{ *this, table, table, "base"}; !tbMgr.SetupRoutes())
        //         return false;
        // }
    }


    // Query Database For Data
    std::vector<std::string> tables = {"students", "teachers", "classes", "permissions"};
    for (const auto& table : tables)
    {
        Log::debug("Table '{}'", table);
        // For each, create table object

        // We need to persist this instance, else it'll be cleaned up causing a crash
        auto tableUnit = std::make_shared<TableUnit>(m_app, table, table, "base");

        if (!tableUnit->setupRoutes())
            return false;

        m_routes.push_back(tableUnit);
    }

    // assert(m_r)

    return true;
}

bool mantis::Router::generateAdminCrudApis()
{
    Log::debug("Mantis::ServerMgr::GenerateAdminCrudApis");

    try
    {
        // We need to persist this instance, else it'll be cleaned up causing a crash
        const auto adminUnit = std::make_shared<TableUnit>(m_app, "__admin", "admin", "auth");
        if (!adminUnit->setupRoutes()) return false;
        m_routes.push_back(adminUnit);

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
