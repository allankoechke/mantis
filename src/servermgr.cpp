#include <mantis/api/servermgr.h>
#include <mantis/mantis.h>

Mantis::ServerMgr::ServerMgr(const MantisApp& app)
    : m_app(make_shared<MantisApp>(app)),
      m_httpServer(std::make_shared<HttpServer>())
{
}

bool Mantis::ServerMgr::GenerateCrudApis()
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

bool Mantis::ServerMgr::GenerateTableCrudApis()
{
    Logger::Debug("Mantis::ServerMgr::GenerateTableCrudApis");

    return true;
}

bool Mantis::ServerMgr::GenerateAdminCrudApis()
{
    Logger::Debug("Mantis::ServerMgr::GenerateAdminCrudApis");

    try
    {
        m_httpServer->Get("/m-admin", [=](const Request& req, Response& res, Context ctx)
        {
            std::cout << req.path << " [START] SUBMIT HANDLER!" << std::endl;
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

        auto must_auth_as_admin = [&](const Request& req, Response& res, Context& ctx)
        {
            Logger::Debug("AdminMiddleware");


            json u;
            u["id"] = "123456789";
            u["name"] = "John Doe";
            ctx.Set<json>("user", u);
            ctx.Set("record", "_users_");
            ctx.Dump();

            Logger::Debug("After Admin Middleware");
            std::cout << std::endl;
            return true;
        };

        // Admin /api/endpoints
        m_httpServer->Get("/api/admin",
                          [&](const Request& req, Response& res, Context& ctx)
                          {
                              Logger::Debug("GET /api/admin Exec: {}", req.path);
                              ctx.Dump();

                              json response;
                              response["status"] = 200;
                              response["message"] = "Admin crud apis";
                              response["data"] = *(ctx.Get<json>("user"));

                              res.status = 200;
                              res.set_content(response.dump(), "application/json");
                              return;
                          }, {must_auth_as_admin});

        m_httpServer->Get("/api/admin/:id",
                          [=](const Request& req, Response& res, Context ctx)
                          {
                              Logger::Debug("GET /api/admin/:id Exec: {}", req.path);

                              std::cout << std::endl;
                              for (auto& [k, v] : req.path_params)
                              {
                                  std::cout << "KV: " << k << "\t" << v << std::endl;
                              }

                              std::cout << "Path Params: " << req.path_params.at("id");
                              Logger::Debug("Exec: {}", req.path);
                              ctx.Dump();

                              json response;
                              response["status"] = 200;
                              response["message"] = "Admin crud apis";
                              response["data"] = json{};

                              res.status = 200;
                              res.set_content(response.dump(), "application/json");
                          }, {must_auth_as_admin});

        m_httpServer->Post("/api/admin",
                           [=](const Request& req, Response& res, Context ctx)
                           {
                               Logger::Debug("POST /api/admin Exec: {}", req.path);
                               ctx.Dump();

                               json response;
                               response["status"] = 200;
                               response["message"] = "Admin crud apis";
                               response["data"] = json{};

                               res.status = 200;
                               res.set_content(response.dump(), "application/json");
                           }, {must_auth_as_admin});

        m_httpServer->Patch("/api/admin/:id",
                            [=](const Request& req, Response& res, Context ctx)
                            {
                                Logger::Debug("PATCH /api/admin/:id Exec: {}", req.path);
                                ctx.Dump();

                                json response;
                                response["status"] = 200;
                                response["message"] = "Admin crud apis";
                                response["data"] = json{};

                                res.status = 200;
                                res.set_content(response.dump(), "application/json");
                            }, {must_auth_as_admin});

        m_httpServer->Delete("/api/admin/:id",
                             [=](const Request& req, Response& res, Context ctx)
                             {
                                 Logger::Debug("DEL /api/admin/:id Exec: {}", req.path);
                                 ctx.Dump();

                                 res.status = 204;
                                 res.set_content(json{}.dump(), "application/json");
                             }, {must_auth_as_admin});
    }

    catch (const std::exception& e)
    {
        Logger::Critical("Error creating admin routes", e.what());
        return false;
    }

    return true;
}

bool Mantis::ServerMgr::AttachUserRoutes()
{
    Logger::Debug("Mantis::ServerMgr::AttachUserRoutes");

    return true;
}
