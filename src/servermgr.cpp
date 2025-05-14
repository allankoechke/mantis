#include <mantis/api/servermgr.h>
#include <mantis/mantis.h>

#include "mantis/api/tablemgr.h"

Mantis::ServerMgr::ServerMgr(const MantisApp& app)
    : m_app(make_shared<MantisApp>(app)),
      m_httpServer(std::make_shared<HttpServer>())
{
}

bool Mantis::ServerMgr::GenerateCrudApis() const
{
    if (!GenerateTableCrudApis())
        return false;

    if (!AttachUserRoutes())
        return false;

    // Add Admin Route as the last, should override
    // any existing route
    if (!GenerateAdminCrudApis())
        return false;

    // If all was completed with no issues,
    // just return OK!
    return true;
}

bool Mantis::ServerMgr::StartListening() const
{
    try
    {
        return m_httpServer->Listen(m_host, m_port);
    }
    catch (const std::exception& e)
    {
        Logger::Critical("Failed to start server: {}", e.what());
    }
    catch (...)
    {
        Logger::Critical("Failed to start server: Unknown Error");
    }

    return false;
}

bool Mantis::ServerMgr::StopListening() const
{
    m_httpServer->Stop();
    return true;
}

std::string Mantis::ServerMgr::Host() const
{
    return m_host;
}

void Mantis::ServerMgr::SetHost(const std::string& host)
{
    m_host = host;
}

int Mantis::ServerMgr::Port() const
{
    return m_port;
}

void Mantis::ServerMgr::SetPort(const int& port)
{
    m_port = port;
}

std::shared_ptr<Mantis::HttpServer> Mantis::ServerMgr::GetHttpServer() const
{
    return m_httpServer;
}

bool Mantis::ServerMgr::GenerateTableCrudApis() const
{
    Logger::Debug("Mantis::ServerMgr::GenerateTableCrudApis");

    // Query Database For Data
    std::vector<std::string> tables = {"students", "teachers", "classes", "permissions"};
    for (const auto& table : tables)
    {
        Logger::Debug("Table '{}'", table);
        // For each, create table object

        if (TableMgr tbMgr{ *this, table, table, "base"}; !tbMgr.SetupRoutes())
            return false;
    }

    return true;
}

bool Mantis::ServerMgr::GenerateAdminCrudApis() const
{
    Logger::Debug("Mantis::ServerMgr::GenerateAdminCrudApis");

    try
    {
        TableMgr tbMgr{ *this, "__admin", "admin", "auth"};
        tbMgr.SetRouteDisplayName("admin");
        if ( !tbMgr.SetupRoutes())
            return false;

        // Setup Admin Dashboard
        m_httpServer->Get("/admin", [=](const Request& req, Response& res, Context ctx)
        {
            Logger::Debug("ServerMgr::GenerateAdminCrudApis for {}", req.path);
            ctx.Dump();

            // Response Object
            json response;

            // Check for correct admin user ...
            const auto admin = ctx.Get<json>("admin");
            if (!admin)
            {
                Logger::Critical("User ID is required for admin endpoints ...");

                response["status"] = "404";
                response["message"] = "Admin with the provided Id was not found!";

                res.status = 404;
                res.reason = "Not Found";
                res.set_content(response, "application/json");
            }

            else
            {
                Logger::Critical("User ID set is = '{}'", admin->dump());
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

                ctx.Set<json>("admin", admin);
                ctx.Dump();

                return true;
            }
        });
    }

    catch (const std::exception& e)
    {
        Logger::Critical("Error creating admin routes: ", e.what());
        return false;
    }

    return true;
}

bool Mantis::ServerMgr::AttachUserRoutes() const
{
    // Logger::Debug("Mantis::ServerMgr::AttachUserRoutes");
    // Just to
    [[maybe_unused]] auto ctx = m_httpServer->Ctx();

    return true;
}
