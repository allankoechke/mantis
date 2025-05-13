//
// Created by allan on 13/05/2025.
//
#include <mantis/api/httpserver.h>
#include <chrono>
#include "mantis/core/logger.h"

thread_local Mantis::Context Mantis::HttpServer::current_context;

void Mantis::Context::Dump()
{
    const auto i = "Context::Dump";
    for (const auto& [key, value] : data)
    {
        if (value.type() == typeid(std::string))
        {
            Logger::Info("{} - {}: {}", i, key, std::any_cast<std::string>(value));
        }
        else if (value.type() == typeid(const char*))
        {
            Logger::Info("{} - {}: {}", i, key, std::any_cast<const char*>(value));
        }
        else if (value.type() == typeid(int))
        {
            Logger::Info("{} - {}: {}", i, key, std::any_cast<int>(value));
        }
        else if (value.type() == typeid(double))
        {
            Logger::Info("{} - {}: {}", i, key, std::any_cast<double>(value));
        }
        else if (value.type() == typeid(float))
        {
            Logger::Info("{} - {}: {}", i, key, std::any_cast<float>(value));
        }
        else if (value.type() == typeid(bool))
        {
            Logger::Info("{} - {}: {}", i, key, (std::any_cast<bool>(value) ? "true" : "false"));
        }
        else if (value.type() == typeid(json))
        {
            Logger::Info("{} - {}: {}", i, key, std::any_cast<json>(value).dump());
        }
        else
        {
            Logger::Info("{} - {}: {}", i, key, "<Unknown Type>");
        }
    }
}

void Mantis::RouteRegistry::Register(const std::string& method,
                                     const std::string& path,
                                     RouteHandlerFunc handler,
                                     const std::vector<Middleware>& middlewares)
{
    routes[{method, path}] = {middlewares, handler};
}

const Mantis::RouteHandler* Mantis::RouteRegistry::Find(const std::string& method, const std::string& path) const
{
    const auto it = routes.find({method, path});
    return it != routes.end() ? &it->second : nullptr;
}

Mantis::HttpServer::HttpServer()
{
    // Set Global Logger
    server.set_logger([](const Request& req, const Response& res)
    {
        const auto start = req.start_time_;
        const auto end = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // std::cout << "ELAPSED: " << duration << "\t" << duration.count() << std::endl;

        // Calculate execution time (if start_time was set)
        auto end_time = std::chrono::steady_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - req.start_time_).count();

        // Log the request details
        // std::cout << req.method << " " << req.path << " - Status: " << res.status
        //     << " - Time: " << duration_ms << "ms" << std::endl;

        Logger::Info("{} {} {}  - Status: {}  - Time: {}ms", req.version, req.method, req.path, res.status,
                     duration_ms);
    });
}

void Mantis::HttpServer::Get(const std::string& path, RouteHandlerFunc handler,
                             std::initializer_list<Middleware> middlewares)
{
    AddRoute([this](const std::string& p, httplib::Server::Handler h)
    {
        server.Get(p, std::move(h));
    }, "GET", path, handler, middlewares);
}

void Mantis::HttpServer::Post(const std::string& path, RouteHandlerFunc handler,
                              std::initializer_list<Middleware> middlewares)
{
    AddRoute([this](const std::string& p, httplib::Server::Handler h)
    {
        server.Post(p, std::move(h));
    }, "POST", path, handler, middlewares);
}

void Mantis::HttpServer::Patch(const std::string& path, RouteHandlerFunc handler,
                               std::initializer_list<Middleware> middlewares)
{
    AddRoute([this](const std::string& p, httplib::Server::Handler h)
    {
        server.Patch(p, std::move(h));
    }, "PATCH", path, handler, middlewares);
}

void Mantis::HttpServer::Delete(const std::string& path, RouteHandlerFunc handler,
                                std::initializer_list<Middleware> middlewares)
{
    AddRoute([this](const std::string& p, httplib::Server::Handler h)
    {
        server.Delete(p, std::move(h));
    }, "DELETE", path, handler, middlewares);
}

bool Mantis::HttpServer::Listen(const std::string& host, const int& port)
{
    Logger::Info("API Endpoints: http://{}:{}/api/", host, port);
    Logger::Info("Admin Dashboard: http://{}:{}/m-admin", host, port);
    return server.listen(host, port);
}

void Mantis::HttpServer::Stop()
{
    Logger::Info("Stopping Server, if its running ...");
    if (server.is_running())
        server.stop();
}

Mantis::Context& Mantis::HttpServer::Ctx()
{
    return current_context;
}

void Mantis::HttpServer::AddRoute(
    MethodBinder bind_method,
    const std::string& method,
    const std::string& path,
    RouteHandlerFunc handler,
    std::initializer_list<Middleware> middlewares)
{
    std::cout << method << " \t'" << path << "'" << std::endl;
    registry.Register(method, path, handler, {middlewares});
    bind_method(path, [this, method, path](const httplib::Request& req, httplib::Response& res)
    {
        current_context = Context();

        const auto* route = registry.Find(method, path);
        if (!route)
        {
            json response;
            response["status"] = 404;
            response["message"] = "Not Found";
            response["data"] = json{};

            res.status = 404;
            res.set_content(response.dump(), "application/json");

            std::cout << "NOT FOUND: " << response.dump() << std::endl;
            return;
        }

        for (const auto& mw : route->middlewares)
        {
            if (!mw(req, res, current_context)) return;
        }

        route->handler(req, res, current_context);
    });
}
