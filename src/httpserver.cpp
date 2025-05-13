//
// Created by allan on 13/05/2025.
//
#include <mantis/api/httpserver.h>

thread_local Mantis::Context Mantis::HttpServer::current_context;

void Mantis::RouteRegistry::Register(const std::string& path, RouteHandlerFunc handler,
    const std::vector<Middleware>& middlewares)
{
    routes[path] = {middlewares, handler};
}

const Mantis::RouteHandler* Mantis::RouteRegistry::Find(const std::string& path) const
{
    const auto it = routes.find(path);
    return it != routes.end() ? &it->second : nullptr;
}

void Mantis::HttpServer::Get(const std::string& path, RouteHandlerFunc handler,
    std::initializer_list<Middleware> middlewares)
{
    AddRoute([this](const std::string& p, httplib::Server::Handler h) {
            server.Get(p, std::move(h));
        }, path, handler, middlewares);
    // Add(static_cast<Method>(&httplib::Server::Get), path, handler, middlewares);
}

void Mantis::HttpServer::Post(const std::string& path, RouteHandlerFunc handler,
    std::initializer_list<Middleware> middlewares)
{
    // Add(static_cast<Method>(&httplib::Server::Post), path, handler, middlewares);
    // Add(reinterpret_cast<Method>(&httplib::Server::Post), path, handler, middlewares);
    AddRoute([this](const std::string& p, httplib::Server::Handler h) {
            server.Post(p, std::move(h));
        }, path, handler, middlewares);
}

void Mantis::HttpServer::Patch(const std::string& path, RouteHandlerFunc handler,
    std::initializer_list<Middleware> middlewares)
{
    // &httplib::Server::Patch
    // Add(reinterpret_cast<Method>(&httplib::Server::Patch), path, handler, middlewares);
    AddRoute([this](const std::string& p, httplib::Server::Handler h) {
            server.Patch(p, std::move(h));
        }, path, handler, middlewares);
}

void Mantis::HttpServer::Delete(const std::string& path, RouteHandlerFunc handler,
    std::initializer_list<Middleware> middlewares)
{
    // Add(reinterpret_cast<Method>(&httplib::Server::Delete), path, handler, middlewares);
    AddRoute([this](const std::string& p, httplib::Server::Handler h) {
            server.Delete(p, std::move(h));
        }, path, handler, middlewares);
}

void Mantis::HttpServer::Listen(const std::string& host, const int& port)
{
    server.listen(host.c_str(), port);
}

Mantis::Context& Mantis::HttpServer::Ctx()
{
    return current_context;
}

void Mantis::HttpServer::Add(Method method, const std::string& path, RouteHandlerFunc handler,
                      std::initializer_list<Middleware> middlewares)
{
    registry.Register(path, handler, {middlewares});
    (server.*method)(path, [this, path](const httplib::Request& req, httplib::Response& res) {
        current_context = Context();
        const auto* route = registry.Find(path);
        if (!route) {
            res.status = 404;
            res.set_content("Route not found", "text/plain");
            return;
        }

        for (const auto& mw : route->middlewares) {
            if (!mw(req, res, current_context)) return;
        }

        route->handler(req, res, current_context);
    });
}

void Mantis::HttpServer::AddRoute(MethodBinder bind_method, const std::string& path, RouteHandlerFunc handler,
    std::initializer_list<Middleware> middlewares)
{
    registry.Register(path, handler, {middlewares});
    bind_method(path, [this, path](const httplib::Request& req, httplib::Response& res) {
        current_context = Context();
        const auto* route = registry.Find(path);
        if (!route) {
            res.status = 404;
            res.set_content("Route not found", "text/plain");
            return;
        }

        for (const auto& mw : route->middlewares) {
            if (!mw(req, res, current_context)) return;
        }

        route->handler(req, res, current_context);
    });
}
