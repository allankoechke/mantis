#include <iostream>
#include <httplib.h>
#include <string>


/*
private members
    - port
    - ip
Router & Middleware
Database

*/

class MantisApp
{
public:
    MantisApp() : svr(std::make_shared<httplib::Server>())
    {

    }
    ~MantisApp() = default;

    bool Start(const std::string& host = "127.0.0.1", const int& port = 8080)
    {
        if (port < 0 || port > 65535)
        {
            std::cerr << "Invalid port number: " << port << std::endl;
            return false;
        }

        m_port = port;
        m_host = host;

        std::cout << "Starting listening on " << m_host << ":" << m_port << std::endl;
        return svr->listen(m_host.c_str(), m_port);
    }

    // Access the server object
    std::shared_ptr<httplib::Server> Server()
    {
        return svr;
    }

private:
    std::shared_ptr<httplib::Server> svr;

    // data dir
    // public dir
    //

    // Server port & host
    int m_port;
    std::string m_host;
};

int main() {
    httplib::Server svr;

    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });

    auto ret = svr.set_mount_point("/", "./www");
    if (!ret) {
        std::cout << "Specified base does not exist yet!" << std::endl;
    }
    
    std::cout << "Starting listening on 0.0.0.0:8080" << std::endl;
    std::cout << svr.listen("0.0.0.0", 8080);


    return 0;
}