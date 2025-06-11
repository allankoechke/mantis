#include "../../include/mantis/core/router.h"

#include "../../include/mantis/utils.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/core/tables/sys_tables.h"


mantis::Router::Router(MantisApp* app)
    : m_app(app),
      m_adminTable(std::make_shared<TableUnit>(m_app, "__admin",
          TableUnit::generateTableId("__admin"), "auth")),
      m_tableRoutes(std::make_shared<SysTablesUnit>(m_app, "__tables",
          TableUnit::generateTableId("__tables"), "base"))
{
    // Override the route display name to easier names. This means that,
    // instead of `<root>/__admin` -> <root>/admins
    // instead of `<root>/__tables` -> <root>/tables
    m_adminTable->setRouteDisplayName("admins");
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

bool mantis::Router::generateAdminCrudApis() const
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

            // Response Object
            json response;
            res.status = 200;
            response["status"] = 200;

            response["data"] = "{}";
            res.set_content(response, "application/json");
        }, {});
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
