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

    const auto sql = m_app->db().session();

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
            const auto tableUnit = std::make_shared<TableUnit>(m_app, schema);
            tableUnit->setTableName(name);
            tableUnit->setTableId(id);

            if (!tableUnit->setupRoutes())
                return false;

            m_routes.push_back(tableUnit);
        }
    }

    return true;
}

bool mantis::Router::generateAdminCrudApis()
{
    Log::trace("Mantis::ServerMgr::GenerateAdminCrudApis");

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
            if (const auto admin = ctx.get<json>("admin"); !admin)
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
    // [[maybe_unused]] auto ctx = m_app->http().context();

    return true;
}
