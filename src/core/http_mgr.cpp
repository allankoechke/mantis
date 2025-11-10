#include "../../include/mantis/core/http_mgr.h"
#include "../../include/mantis/core/logs_mgr.h"
#include "../../include/mantis/mantisbase.h"
#include "../../include/mantis/core/private-impl/duktape_custom_types.h"

#include <chrono>
#include <thread>

// For thread logger
#include <spdlog/sinks/stdout_color_sinks-inl.h>
#include <spdlog/sinks/ansicolor_sink.h>

#define __file__ "core/http.cpp"

namespace mantis
{
    size_t RouteKeyHash::operator()(const RouteKey& k) const
    {
        return std::hash<std::string>()(k.first + "#" + k.second);
    }

    void RouteRegistry::add(const std::string& method,
                            const std::string& path,
                            const RouteHandlerFunc handler,
                            const std::vector<MiddlewareFunc>& middlewares)
    {
        routes[{method, path}] = {middlewares, handler};
    }

    void RouteRegistry::add(const std::string& method,
                            const std::string& path,
                            const RouteHandlerFuncWithContentReader handler,
                            const std::vector<MiddlewareFunc>& middlewares)
    {
        routes[{method, path}] = {middlewares, handler};
    }

    const RouteHandler* RouteRegistry::find(const std::string& method, const std::string& path) const
    {
        const auto it = routes.find({method, path});
        return it != routes.end() ? &it->second : nullptr;
    }

    json RouteRegistry::remove(const std::string& method, const std::string& path)
    {
        json res;
        res["error"] = "";

        const auto it = routes.find({method, path});
        if (it == routes.end())
        {
            const auto err = std::format("Route for {} {} not found!", method, path);
            // We didn't find that route, return error
            res["error"] = err;
            logger::warn("{}", err);
            return res;
        }

        // Remove item found at the iterator
        routes.erase(it);
        logger::info("Route for {} {} erased!", method, path);
        return res;
    }

    HttpMgr::HttpMgr()
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

        svr.set_logger([](const auto& req, const auto& res)
        {
            // Calculate execution time (if start_time was set)
            const auto end_time = std::chrono::steady_clock::now();
            const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - req.start_time_).count();

            if (res.status < 400)
            {
                logger::info("{} {:<7} {}  - Status: {}  - Time: {}ms",
                          req.version, req.method, req.path, res.status, duration_ms);
            }
            else
            {
                // Decompress if content is compressed
                if (res.body.empty())
                {
                    logger::info("{} {:<7} {}  - Status: {}  - Time: {}ms",
                              req.version, req.method, req.path, res.status, duration_ms);
                }
                else
                {
                    // Get the compression encoding
                    const std::string encoding = res.get_header_value("Content-Encoding");

                    auto body = encoding.empty() ? res.body : decompressResponseBody(res.body, encoding);
                    logger::info("{} {:<7} {}  - Status: {}  - Time: {}ms\n\t└──Body: {}",
                              req.version, req.method, req.path, res.status, duration_ms, body);
                }
            }
        });

        // Handle preflight OPTIONS requests
        svr.Options(".*", [](const auto& req, auto& res)
        {
            // Headers are already set by post_routing_handler
            res.status = 200;
        });

        // Set Error Handler
        svr.set_error_handler([]([[maybe_unused]] const httplib::Request& req, httplib::Response& res)
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

    HttpMgr::~HttpMgr()
    {
        if (svr.is_running()) svr.stop();
    }

    void HttpMgr::Get(const std::string& path,
                       const RouteHandlerFunc& handler,
                       const std::initializer_list<MiddlewareFunc> middlewares)
    {
        route([this](const std::string& p, httplib::Server::Handler h)
        {
            svr.Get(p, std::move(h));
        }, "GET", path, handler, middlewares);
    }

    void HttpMgr::Post(const std::string& path,
                        const RouteHandlerFunc& handler,
                        const std::initializer_list<MiddlewareFunc> middlewares)
    {
        route([this](const std::string& p, httplib::Server::Handler h)
        {
            svr.Post(p, std::move(h));
        }, "POST", path, handler, middlewares);
    }

    void HttpMgr::Post(const std::string& path,
                        const RouteHandlerFuncWithContentReader& handler,
                        const std::initializer_list<MiddlewareFunc> middlewares)
    {
        route([this](const std::string& p, httplib::Server::HandlerWithContentReader h)
        {
            svr.Post(p, std::move(h));
        }, "POST", path, handler, middlewares);
    }

    void HttpMgr::Patch(const std::string& path,
                         const RouteHandlerFunc& handler,
                         const std::initializer_list<MiddlewareFunc> middlewares)
    {
        route([this](const std::string& p, httplib::Server::Handler h)
        {
            svr.Patch(p, std::move(h));
        }, "PATCH", path, handler, middlewares);
    }

    void HttpMgr::Patch(const std::string& path,
                         const RouteHandlerFuncWithContentReader& handler,
                         const std::initializer_list<MiddlewareFunc> middlewares)
    {
        route([this](const std::string& p, httplib::Server::HandlerWithContentReader h)
        {
            svr.Patch(p, std::move(h));
        }, "PATCH", path, handler, middlewares);
    }

    void HttpMgr::Delete(const std::string& path,
                          const RouteHandlerFunc& handler,
                          const std::initializer_list<MiddlewareFunc> middlewares)
    {
        route([this](const std::string& p, httplib::Server::Handler h)
        {
            svr.Delete(p, std::move(h));
        }, "DELETE", path, handler, middlewares);
    }

    bool HttpMgr::listen(const std::string& host, const int& port)
    {
        // Check if server can bind to port before launching
        if (!svr.is_valid())
        {
            logger::critical("Server is not valid. Maybe port is in use or permissions issue.\n");
            return false;
        }

        // Launch logging/browser in separate thread after listen starts
        std::thread notifier([host, port]() -> void
        {
            // Wait a little for the server to be fully ready
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            auto endpoint = std::format("{}:{}", host, port);

            auto t_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            t_sink->set_level(spdlog::level::trace);
            t_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%-8l] %v");

            spdlog::logger logger("t_sink", {t_sink});
            logger.set_level(spdlog::level::trace);

            logger.info(
                "Starting Servers: \n\t├── API Endpoints: http://{}/api/v1/ \n\t└── Admin Dashboard: http://{}/admin\n",
                endpoint, endpoint);

            MantisBase::instance().openBrowserOnStart();
        });

        if (!svr.listen(host, port))
        {
            logger::critical("Error: Failed to bind to {}:{}", host, port);
            notifier.join();
            return false;
        }

        notifier.join();
        return true;
    }

    void HttpMgr::close()
    {
        if (svr.is_running())
        {
            svr.stop();
            logger::info("HTTP Server Stopped.\n\t ...");
        }
    }

    RouteRegistry& HttpMgr::routeRegistry()
    {
        return registry;
    }

    httplib::Server& HttpMgr::server()
    {
        return svr;
    }

    std::string HttpMgr::hashMultipartMetadata(const httplib::FormData& data)
    {
        constexpr std::hash<std::string> hasher;
        const size_t h1 = hasher(data.name);
        const size_t h3 = hasher(data.filename);
        const size_t h4 = hasher(data.content_type);
        const size_t content_size_hash = hasher(std::to_string(data.content.size()));

        size_t result = h1;
        result ^= h3 + 0x9e3779b9 + (result << 6) + (result >> 2);
        result ^= h4 + 0x9e3779b9 + (result << 6) + (result >> 2);
        result ^= content_size_hash + 0x9e3779b9 + (result << 6) + (result >> 2);

        return std::to_string(result);
    }


#ifdef MANTIS_ENABLE_SCRIPTING
    void HttpUnit::registerDuktapeMethods()
    {
        const auto ctx = MantisApp::instance().ctx();

        dukglue_register_method(ctx, &HttpUnit::close, "close");

        MantisResponse::registerDuktapeMethods();
        MantisRequest::registerDuktapeMethods();
    }
#endif


    std::string HttpMgr::decompressResponseBody(const std::string& body, const std::string& encoding)
    {
        std::string decompressed_content;

        if (encoding == "gzip" || encoding == "deflate")
        {
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
            httplib::detail::gzip_decompressor decompressor;
            if (decompressor.is_valid())
            {
                decompressor.decompress(
                    body.data(), body.size(),
                    [&](const char* data, const size_t len)
                    {
                        decompressed_content.append(data, len);
                        return true;
                    }
                );
            }
#endif
        }
        else if (encoding.find("br") != std::string::npos)
        {
#ifdef CPPHTTPLIB_BROTLI_SUPPORT
            httplib::detail::brotli_decompressor decompressor;
            if (decompressor.is_valid())
            {
                decompressor.decompress(
                    body.data(), body.size(),
                    [&](const char* data, const size_t len)
                    {
                        decompressed_content.append(data, len);
                        return true;
                    }
                );
            }
#endif
        }
        else if (encoding == "zstd")
        {
#ifdef CPPHTTPLIB_ZSTD_SUPPORT
            httplib::detail::zstd_decompressor decompressor;
            if (decompressor.is_valid())
            {
                decompressor.decompress(
                    body.data(), body.size(),
                    [&](const char* data, const size_t len)
                    {
                        decompressed_content.append(data, len);
                        return true;
                    }
                );
            }
#endif
        }

        return decompressed_content;
    }

    void HttpMgr::route(
        const MethodBinder<httplib::Server::Handler>& bind_method,
        const std::string& method,
        const std::string& path,
        const RouteHandlerFunc& handler,
        std::initializer_list<MiddlewareFunc> middlewares)
    {
        registry.add(method, path, handler, {middlewares});
        bind_method(path, [this, method, path](const httplib::Request& req, httplib::Response& res)
        {
            MantisRequest ma_req{req};
            MantisResponse ma_res{res};

            const auto* route = registry.find(method, path);
            if (!route)
            {
                json response;
                response["status"] = 404;
                response["error"] = std::format("{} {} Route Not Found", method, path);
                response["data"] = json::object();

                ma_res.sendJson(404, response);
                return;
            }

            for (const auto& mw : route->middlewares)
            {
                if (!mw(ma_req, ma_res)) return;
            }

            if (const auto func = std::get_if<RouteHandlerFunc>(&route->handler))
            {
                (*func)(ma_req, ma_res);
            }
        });
    }

    void HttpMgr::route(
        const MethodBinder<httplib::Server::HandlerWithContentReader>& bind_method,
        const std::string& method, const std::string& path,
        const RouteHandlerFuncWithContentReader& handler,
        std::initializer_list<MiddlewareFunc> middlewares)
    {
        registry.add(method, path, handler, {middlewares});
        bind_method(path, [this, method, path](const httplib::Request& req,
                                               httplib::Response& res,
                                               const MantisContentReader& content_reader)
                    {
                        MantisRequest ma_req{req};
                        MantisResponse ma_res{res};

                        const auto* route = registry.find(method, path);
                        if (!route)
                        {
                            json response;
                            response["status"] = 404;
                            response["error"] = std::format("{} {} Route Not Found", method, path);
                            response["data"] = json::object();

                            ma_res.sendJson(404, response);
                            return;
                        }

                        for (const auto& mw : route->middlewares)
                        {
                            if (!mw(ma_req, ma_res)) return;
                        }

                        if (const auto func = std::get_if<RouteHandlerFuncWithContentReader>(&route->handler))
                        {
                            (*func)(ma_req, ma_res, content_reader);
                        }
                    }
        );
    }
}
