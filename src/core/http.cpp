//
// Created by allan on 13/05/2025.
//
#include "../../include/mantis/core/http.h"
#include "../../include/mantis/core/logging.h"
#include "../../include/mantis/app/app.h"

#include <chrono>
#include <thread>
#include <format>

#define __file__ "core/http.cpp"

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

void mantis::RouteRegistry::add(const std::string& method, const std::string& path,
                                RouteHandlerFuncWithContentReader handler, const std::vector<Middleware>& middlewares)
{
    routes[{method, path}] = {middlewares, handler};
}

const mantis::RouteHandler* mantis::RouteRegistry::find(const std::string& method, const std::string& path) const
{
    const auto it = routes.find({method, path});
    return it != routes.end() ? &it->second : nullptr;
}

mantis::json mantis::RouteRegistry::remove(const std::string& method, const std::string& path)
{
    json res;
    res["error"] = "";

    const auto it = routes.find({method, path});
    if (it == routes.end())
    {
        // We didn't find that route, return error
        res["error"] = "Route for " + method + " " + path + " not found!";
        Log::warn("Route for {} {} not found!", method, path);
        return res;
    }

    // Remove item found at the iterator
    routes.erase(it);
    Log::info("Route for {} {} erased!", method, path);
    return res;
}

mantis::HttpUnit::HttpUnit()
{
    // Let's fix timing initialization, set the start time to current time
    svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res)
    {
        auto& mutable_req = const_cast<httplib::Request&>(req);
        mutable_req.start_time_ = std::chrono::steady_clock::now(); // Set the start time
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // Add CORS headers to all responses
    svr.set_post_routing_handler([](const auto& req, auto& res)
    {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PATCH, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Max-Age", "86400");
    });

    // Handle preflight OPTIONS requests
    svr.Options(".*", [](const auto& req, auto& res)
    {
        // Headers are already set by post_routing_handler
        res.status = 200;
    });

    // Set Global Logger
    svr.set_logger([](const Request& req, const Response& res)
    {
        // Calculate execution time (if start_time was set)
        const auto end_time = std::chrono::steady_clock::now();
        const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - req.start_time_).count();

        if (res.status < 400)
        {
            Log::info("{} {:<7} {}  - Status: {}  - Time: {}ms",
                      req.version, req.method, req.path, res.status, duration_ms);
        }
        else
        {
            Log::info("{} {:<7} {}  - Status: {}  - Time: {}ms\n\tData: {}",
                      req.version, req.method, req.path, res.status, duration_ms, res.body);
        }
    });

    // Set Error Handler
    svr.set_error_handler([]([[maybe_unused]] const Request& req, Response& res)
    {
        if (res.body.empty())
        {
            json response;
            response["status"] = res.status;
            response["data"] = json::object();

            if (res.status == 404)
                response["error"] = "Resource not found!";
            else if (res.status >= 500)
                response["error"] = "Internal server error, try again later!";
            else
                response["error"] = "Something went wrong here!";

            res.set_content(response.dump(), "application/json");
        }
    });
}

void mantis::HttpUnit::Get(const std::string& path, const RouteHandlerFunc& handler,
                           const std::initializer_list<Middleware> middlewares)
{
    route([this](const std::string& p, httplib::Server::Handler h)
    {
        svr.Get(p, std::move(h));
    }, "GET", path, handler, middlewares);
}

void mantis::HttpUnit::Post(const std::string& path, const RouteHandlerFunc& handler,
                            const std::initializer_list<Middleware> middlewares)
{
    route([this](const std::string& p, httplib::Server::Handler h)
    {
        svr.Post(p, std::move(h));
    }, "POST", path, handler, middlewares);
}

void mantis::HttpUnit::Post(const std::string& path, const RouteHandlerFuncWithContentReader& handler,
                            const std::initializer_list<Middleware> middlewares)
{
    route([this](const std::string& p, httplib::Server::HandlerWithContentReader h)
    {
        svr.Post(p, std::move(h));
    }, "POST", path, handler, middlewares);
}

void mantis::HttpUnit::Patch(const std::string& path, const RouteHandlerFunc& handler,
                             const std::initializer_list<Middleware> middlewares)
{
    route([this](const std::string& p, httplib::Server::Handler h)
    {
        svr.Patch(p, std::move(h));
    }, "PATCH", path, handler, middlewares);
}

void mantis::HttpUnit::Patch(const std::string& path, const RouteHandlerFuncWithContentReader& handler,
                             const std::initializer_list<Middleware> middlewares)
{
    route([this](const std::string& p, httplib::Server::HandlerWithContentReader h)
    {
        svr.Patch(p, std::move(h));
    }, "PATCH", path, handler, middlewares);
}

void mantis::HttpUnit::Delete(const std::string& path, const RouteHandlerFunc& handler,
                              const std::initializer_list<Middleware> middlewares)
{
    route([this](const std::string& p, httplib::Server::Handler h)
    {
        svr.Delete(p, std::move(h));
    }, "DELETE", path, handler, middlewares);
}

bool mantis::HttpUnit::listen(const std::string& host, const int& port)
{
    std::cout << std::endl;

    // Check if server can bind to port before launching
    if (!svr.is_valid())
    {
        Log::critical("Server is not valid. Maybe port is in use or permissions issue.\n");
        return false;
    }

    // Launch logging/browser in separate thread after listen starts
    std::thread notifier([=]()
    {
        // Wait a little for the server to be fully ready
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto endpoint = std::format("{}:{}", host, port);
        Log::info("Starting Servers: \n\t├── API Endpoints: http://{}/api/v1/ \n\t└── Admin Dashboard: http://{}/admin",
                  endpoint, endpoint);

        MantisApp::instance().openBrowserOnStart();

        std::cout << std::endl;
    });

    if (!svr.listen(host, port))
    {
        Log::critical("Error: Failed to bind to {}:{}", host, port);
        notifier.join();
        return false;
    }

    notifier.join();
    return true;
}

void mantis::HttpUnit::close()
{
    Log::trace("Stopping Server, if its running ...");
    if (svr.is_running())
        svr.stop();
}

mantis::Context& mantis::HttpUnit::context()
{
    return current_context;
}

mantis::RouteRegistry& mantis::HttpUnit::routeRegistry()
{
    return registry;
}

httplib::Server& mantis::HttpUnit::server()
{
    return svr;
}

void mantis::HttpUnit::route(
    const MethodBinder<httplib::Server::Handler>& bind_method,
    const std::string& method,
    const std::string& path,
    const RouteHandlerFunc& handler,
    std::initializer_list<Middleware> middlewares)
{
    registry.add(method, path, handler, {middlewares});
    bind_method(path, [this, method, path](const httplib::Request& req, httplib::Response& res)
    {
        current_context = Context{};

        const auto* route = registry.find(method, path);
        if (!route)
        {
            json response;
            response["status"] = 404;
            response["error"] = std::format("{} {} Route Not Found", method, path);
            response["data"] = json::object();

            res.status = 404;
            res.set_content(response.dump(), "application/json");
            return;
        }

        for (const auto& mw : route->middlewares)
        {
            if (!mw(req, res, current_context)) return;
        }

        // Create empty dummy ContentReader
        static httplib::ContentReader empty_content_reader(
            // Empty regular content reader - does nothing
            [](httplib::ContentReceiver receiver) -> bool
            {
                return true; // Just return success without calling receiver
            },
            // Empty multipart content reader - does nothing
            [](httplib::MultipartContentHeader header, httplib::ContentReceiver receiver) -> bool
            {
                return true; // Just return success without calling header or receiver
            }
        );

        std::visit(
            overloaded{
                [&](const RouteHandlerFunc& f)
                {
                    f(req, res, current_context);
                },
                [&](const RouteHandlerFuncWithContentReader& f)
                {
                    f(req, res, empty_content_reader, current_context);
                }
            }, route->handler
        );
    });
}

void mantis::HttpUnit::route(
    const MethodBinder<httplib::Server::HandlerWithContentReader>& bind_method,
    const std::string& method, const std::string& path,
    const RouteHandlerFuncWithContentReader& handler,
    std::initializer_list<Middleware> middlewares)
{
    registry.add(method, path, handler, {middlewares});
    bind_method(
        path,
        [this, method, path](const httplib::Request& req, httplib::Response& res,
                             const httplib::ContentReader& content_reader)
        {
            current_context = Context{};

            const auto* route = registry.find(method, path);
            if (!route)
            {
                json response;
                response["status"] = 404;
                response["error"] = std::format("{} {} Route Not Found", method, path);
                response["data"] = json::object();

                res.status = 404;
                res.set_content(response.dump(), "application/json");
                return;
            }

            for (const auto& mw : route->middlewares)
            {
                if (!mw(req, res, current_context)) return;
            }

            std::visit(
                overloaded{
                    [&](const RouteHandlerFunc& f)
                    {
                        f(req, res, current_context);
                    },
                    [&](const RouteHandlerFuncWithContentReader& f)
                    {
                        f(req, res, content_reader, current_context);
                    }
                }, route->handler
            );
        }
    );
}
