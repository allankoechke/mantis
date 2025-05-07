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