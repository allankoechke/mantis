#include <mantis/api/servermgr.h>
#include <mantis/mantis.h>

Mantis::ServerMgr::ServerMgr(const MantisApp& app)
    : m_app(make_shared<MantisApp>(app)),
      m_httpServer(std::make_shared<HttpServer>())
{
}

bool Mantis::ServerMgr::GenerateCrudApis()
{
    // Fetch All Schema & Tables

    // Create API endpoints for different tables

    // Create API endpoints for Admins
    // m_app->Server()->Get("/_", [](const httplib::Request&, httplib::Response& res)
    // {
    //     res.set_content("Hello World!", "text/plain");
    // });

    m_httpServer->Post("/submit", [=](const Request& req, Response& res, Context ctx)
    {
        std::cout << req.path << "SUBMIT HANDLER!" << std::endl;
    }, {
        [=](const Request& req, Response& res, Context ctx)->bool
            {
                std::cout << req.path << "SUBMIT HANDLER MIDDLEWARE!" << std::endl;
                return true;
            }
    });

    // If all is OK, just return OK!
    return true;
}

void Mantis::ServerMgr::SetHost(const std::string& host)
{
    m_host = host;
}

bool Mantis::ServerMgr::StartListening()
{
    return true;
}

bool Mantis::ServerMgr::StopListening()
{
    // if (m_svrMgr->is_running())
    //     m_svrMgr->stop();
    return 0;

}

std::string Mantis::ServerMgr::Host() const
{
    return m_host;
}

void Mantis::ServerMgr::SetPort(const int& port)
{
    m_port = port;
}

int Mantis::ServerMgr::Port() const
{
    return m_port;
}
