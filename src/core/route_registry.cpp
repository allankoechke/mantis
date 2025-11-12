#include "../../include/mantis/core/route_registry.h"
#include "../../include/mantis/core/logs_mgr.h"
#include "../../include/mantis/mantisbase.h"
#include "../../include/mantis/core/http.h"
#include "../../include/mantis/core/http.h"


#define __file__ "core/http.cpp"

namespace mantis
{
    size_t RouteKeyHash::operator()(const RouteKey& k) const
    {
        return std::hash<std::string>()(k.first + "#" + k.second);
    }

    void RouteRegistry::add(const std::string& method,
                            const std::string& path,
                            const HandlerFn handler,
                            const Middlewares& middlewares)
    {
        routes[{method, path}] = {middlewares, handler};
    }

    void RouteRegistry::add(const std::string& method,
                            const std::string& path,
                            const HandlerWithContentReaderFn handler,
                            const Middlewares& middlewares)
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
}
