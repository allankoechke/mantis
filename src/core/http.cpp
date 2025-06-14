//
// Created by allan on 13/05/2025.
//
#include "../../include/mantis/core/http.h"
#include "../../include/mantis/core/logging.h"

#include <chrono>

thread_local mantis::Context mantis::HttpUnit::current_context;

void mantis::Context::dump()
{
    for (const auto& [key, value] : data)
    {
        const auto i = "Context::Dump";
        if (value.type() == typeid(std::string))
        {
            Log::debug("{} - {}: {}", i, key, std::any_cast<std::string>(value));
        }
        else if (value.type() == typeid(const char*))
        {
            Log::debug("{} - {}: {}", i, key, std::any_cast<const char*>(value));
        }
        else if (value.type() == typeid(int))
        {
            Log::debug("{} - {}: {}", i, key, std::any_cast<int>(value));
        }
        else if (value.type() == typeid(double))
        {
            Log::debug("{} - {}: {}", i, key, std::any_cast<double>(value));
        }
        else if (value.type() == typeid(float))
        {
            Log::debug("{} - {}: {}", i, key, std::any_cast<float>(value));
        }
        else if (value.type() == typeid(bool))
        {
            Log::debug("{} - {}: {}", i, key, (std::any_cast<bool>(value) ? "true" : "false"));
        }
        else if (value.type() == typeid(json))
        {
            Log::debug("{} - {}: {}", i, key, std::any_cast<json>(value).dump());
        }
        else
        {
            Log::debug("{} - {}: {}", i, key, "<Unknown Type>");
        }
    }
}

size_t mantis::RouteKeyHash::operator()(const RouteKey& k) const
{
    return std::hash<std::string>()(k.first + "#" + k.second);
}

void mantis::RouteRegistry::add(const std::string& method,
                                     const std::string& path,
                                     RouteHandlerFunc handler,
                                     const std::vector<Middleware>& middlewares)
{
    routes[{method, path}] = {middlewares, handler};
}

const mantis::RouteHandler* mantis::RouteRegistry::find(const std::string& method, const std::string& path) const
{
    const auto it = routes.find({method, path});
    return it != routes.end() ? &it->second : nullptr;
}

mantis::HttpUnit::HttpUnit()
{
    // Let's fix timing initialization, set the start time to current time
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

        Log::info("{} {:<7} {}  - Status: {}  - Time: {}ms", req.version, req.method, req.path, res.status,
                     duration_ms);
    });

    // Set Error Handler
    server.set_error_handler([]([[maybe_unused]] const Request& req, Response& res)
    {
        if (res.status == 404)
        {
            json response;
            response["status"] = 404;
            response["message"] = "Resource Not Found";
            response["data"] = json::object();

            res.status = 404;
            res.set_content(response.dump(), "application/json");
        }
    });
}

void mantis::HttpUnit::Get(const std::string& path, RouteHandlerFunc handler,
                             std::initializer_list<Middleware> middlewares)
{
    route([this](const std::string& p, httplib::Server::Handler h)
    {
        server.Get(p, std::move(h));
    }, "GET", path, handler, middlewares);
}

void mantis::HttpUnit::Post(const std::string& path, RouteHandlerFunc handler,
                              std::initializer_list<Middleware> middlewares)
{
    route([this](const std::string& p, httplib::Server::Handler h)
    {
        server.Post(p, std::move(h));
    }, "POST", path, handler, middlewares);
}

void mantis::HttpUnit::Patch(const std::string& path, RouteHandlerFunc handler,
                               std::initializer_list<Middleware> middlewares)
{
    route([this](const std::string& p, httplib::Server::Handler h)
    {
        server.Patch(p, std::move(h));
    }, "PATCH", path, handler, middlewares);
}

void mantis::HttpUnit::Delete(const std::string& path, RouteHandlerFunc handler,
                                std::initializer_list<Middleware> middlewares)
{
    route([this](const std::string& p, httplib::Server::Handler h)
    {
        server.Delete(p, std::move(h));
    }, "DELETE", path, handler, middlewares);
}

bool mantis::HttpUnit::listen(const std::string& host, const int& port)
{
    Log::info("API Endpoints: http://{}:{}/api/v1/", host, port);
    Log::info("Admin Dashboard: http://{}:{}/admin", host, port);
    return server.listen(host, port);
}

void mantis::HttpUnit::close()
{
    Log::trace("Stopping Server, if its running ...");
    if (server.is_running())
        server.stop();
}

mantis::Context& mantis::HttpUnit::context()
{
    return current_context;
}

void mantis::HttpUnit::route(
    MethodBinder bind_method,
    const std::string& method,
    const std::string& path,
    RouteHandlerFunc handler,
    std::initializer_list<Middleware> middlewares)
{
    registry.add(method, path, handler, {middlewares});
    bind_method(path, [this, method, path](const httplib::Request& req, httplib::Response& res)
    {
        current_context = Context();

        const auto* route = registry.find(method, path);
        if (!route)
        {
            json response;
            response["status"] = 404;
            response["message"] = "Resource Not Found";
            response["data"] = json::object();

            res.status = 404;
            res.set_content(response.dump(), "application/json");
            return;
        }

        for (const auto& mw : route->middlewares)
        {
            if (!mw(req, res, current_context)) return;
        }

        route->handler(req, res, current_context);
    });
}
