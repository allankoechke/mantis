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

size_t Mantis::RouteKeyHash::operator()(const RouteKey& k) const
{
    return std::hash<std::string>()(k.first + "#" + k.second);
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
    // Lets fix timing initialization, set the start time to current time
    server.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res)
    {
        auto& mutable_req = const_cast<httplib::Request&>(req);
        mutable_req.start_time_ = std::chrono::steady_clock::now(); // Set the start time
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // Set Global Logger
    server.set_logger([](const Request& req, const Response& res)
    {
        // Calculate execution time (if start_time was set)
        const auto end_time = std::chrono::steady_clock::now();
        const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - req.start_time_).count();

        Logger::Info("{} {:<7} {}  - Status: {}  - Time: {}ms", req.version, req.method, req.path, res.status,
                     duration_ms);
    });

    // Set Error Handler
    server.set_error_handler([]([[maybe_unused]] const Request& req, Response& res)
    {
        if (res.status == 404)
        {
            json response;
            response["status"] = 404;
            response["message"] = "Not Found";
            response["data"] = json{};

            res.status = 404;
            res.set_content(response.dump(), "application/json");
        }
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
