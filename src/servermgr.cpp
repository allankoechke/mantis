#include <mantis/api/servermgr.h>
#include <mantis/mantis.h>

//httplib::Server svr;
//
//svr.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
//    res.set_content("Hello World!", "text/plain");
//});
//
//auto ret = svr.set_mount_point("/", "./www");
//if (!ret) {
//    std::cout << "Specified base does not exist yet!" << std::endl;
//}
//
//std::cout << "Starting listening on 0.0.0.0:8080" << std::endl;
//std::cout << svr.listen("0.0.0.0", 8080);
Mantis::ServerMgr::ServerMgr(const MantisApp& app)
    : m_app(make_shared<MantisApp>(app))
{
}

bool Mantis::ServerMgr::GenerateCrudApis()
{
    // Fetch All Schema & Tables

    // Create API endpoints for different tables

    // Create API endpoints for Admins
    m_app->Server()->Get("/_", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });

    // If all is OK, just return OK!
    return true;
}
