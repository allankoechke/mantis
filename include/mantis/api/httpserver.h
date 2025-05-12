//
// Created by allan on 12/05/2025.
//

#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <httplib.h>
#include <unordered_map>
#include <any>
#include <functional>
#include <string>
#include <vector>

namespace Mantis
{
    class Context
    {
        std::unordered_map<std::string, std::any> data;

    public:
        template <typename T>
        void Set(const std::string& key, T value)
        {
            data[key] = std::move(value);
        }

        template <typename T>
        T* Get(const std::string& key)
        {
            auto it = data.find(key);
            if (it != data.end()) return std::any_cast<T>(&it->second);
            return nullptr;
        }
    };

    using Middleware = std::function<bool(const httplib::Request&, httplib::Response&, Context&)>;
    using RouteHandlerFunc = std::function<void(const httplib::Request&, httplib::Response&, Context&)>;

    struct RouteHandler
    {
        std::vector<Middleware> middlewares;
        RouteHandlerFunc handler;
    };

    class RouteRegistry
    {
        std::unordered_map<std::string, RouteHandler> routes;

    public:
        void Register(const std::string& path,
                      RouteHandlerFunc handler,
                      const std::vector<Middleware>& middlewares);

        const RouteHandler* Find(const std::string& path) const;
    };

    class HttpServer
    {
    public:
        void Get(const std::string& path,
                 RouteHandlerFunc handler,
                 std::initializer_list<Middleware> middlewares = {});

        void Post(const std::string& path,
                  RouteHandlerFunc handler,
                  std::initializer_list<Middleware> middlewares = {});

        void Patch(const std::string& path,
                   RouteHandlerFunc handler,
                   std::initializer_list<Middleware> middlewares = {});

        void Delete(const std::string& path,
                    RouteHandlerFunc handler,
                    std::initializer_list<Middleware> middlewares = {});

        void Listen(const std::string& host, const int& port);

        static Context& Ctx();

    private:
        using Method = void (httplib::Server::*)(const std::string&, httplib::Server::Handler);

        void Add(Method method,
                 const std::string& path,
                 RouteHandlerFunc handler,
                 std::initializer_list<Middleware> middlewares);


        httplib::Server server;
        RouteRegistry registry;
        thread_local static Context current_context;
    };
}

#endif // HTTPSERVER_H
