/**
 * @file http.h
 * @brief Wrapper around http-lib to add middleware support and context values to be passed from middlewares
 * up to the last handler.
 *
 * Created by allan on 12/05/2025.
 */

#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <httplib.h>
#include <unordered_map>
#include <any>
#include <functional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "logging.h"

#define REQUEST_HANDLED false
#define REQUEST_PENDING true

namespace mantis
{
    /// Shorten JSON namespace
    using json = nlohmann::json;

    /**
     * The `Context` class provides a means to set/get a key-value data that can be shared uniquely between middlewares
     * and the handler functions. This allows sending data down the chain from the first to the last handler.
     *
     * For instance, the auth middleware will inject user `id` and subsequent middlewares can retrieve it as needed.
     *
     * @code
     * // Create the object
     * Context ctx;
     *
     * // Add values
     * ctx.set<std::string>("key", "Value");
     * ctx.set<int>("id", 967567);
     * ctx.set<bool>("verified", true);
     *
     * // Retrieve values
     * std::optional key = ctx.get<std::string>("key");
     * @endcode
     *
     * The value returned from the `get()` is a std::optional, meaning a std::nullopt if the key was not found.
     * @code
     * std::optional key = ctx.get<std::string>("key");
     * if(key.has_value()) { .... }
     * @endcode
     *
     * Additionally, we have a @see get_or() method that takes in a key and a default value if the key is missing. This
     * unlike @see get() method, returns a `T&` instead of `T*` depending on the usage needs.
     */
    class Context
    {
        std::unordered_map<std::string, std::any> data;
        std::string __class_name__ = "mantis::Context";

    public:
        Context() = default;
        /**
         * @brief Convenience method for dumping context data for debugging.
         */
        void dump();

        /**
         * @brief Store a key-value data in the context
         *
         * @tparam T Value data type
         * @param key Value key
         * @param value Value to be stored
         */
        template <typename T>
        void set(const std::string& key, T value)
        {
            data[key] = std::move(value);
        }

        /**
         * @brief Get context value given the key.
         *
         * @tparam T Value data type
         * @param key Value key
         * @return Value wrapped in a std::optional
         */
        template <typename T>
        std::optional<T*> get(const std::string& key)
        {
            const auto it = data.find(key);
            if (it != data.end()) return std::any_cast<T>(&it->second);
            return std::nullopt;
        }

        /**
         * @brief Get context value given the key.
         *
         * @tparam T Value data type
         * @param key Value key
         * @param default_value Default value if key is missing
         * @return Value or default value
         */
        template <typename T>
        T& get_or(const std::string& key, T default_value)
        {
            if (const auto it = data.find(key); it == data.end())
            {
                data[key] = std::move(default_value);
            }
            return std::any_cast<T&>(data.at(key));
        }
    };

    ///> Shorthand for httplib::Request
    using Request = httplib::Request;

    ///> Shorthand for httplib::Response
    using Response = httplib::Response;

    ///> Shorthand for httplib::ContentReader
    using ContentReader = httplib::ContentReader;

    ///> Middleware shorthand for the function
    using Middleware = std::function<bool(const Request&, Response&, Context&)>;

    ///> Route Handler function shorthand
    using RouteHandlerFunc = std::function<void(const httplib::Request&, httplib::Response&, Context&)>;

    ///> Route Handler function with content reader shorthand
    using RouteHandlerFuncWithContentReader = std::function<void(const httplib::Request&, httplib::Response&, const httplib::ContentReader&, Context&)>;

    ///> Syntactic sugar for request method which is a std::string
    using Method = std::string;

    ///> Syntactic sugar for request path which is a std::string
    using Path = std::string;

    ///> Shorthand notation for the request's method, path pair.
    using RouteKey = std::pair<Method, Path>;

    /**
     * Structure to allow for hashing of the `RouteKey` for use in std::unordered_map as a key.
     */
    struct RouteKeyHash
    {
        /**
         * @brief Operator function called when hashing RouteKey is required.
         * @param k RouteKey pair
         * @return Hash of RouteKey
         */
        size_t operator()(const RouteKey& k) const;
    };

    /**
     * @brief Struct encompassing the list of middlewares and the handler function registered to a specific route.
     */
    struct RouteHandler
    {
        std::vector<Middleware> middlewares; ///> List of @see Middlewares for a route.
        std::variant<RouteHandlerFunc, RouteHandlerFuncWithContentReader> handler; ///> Handler function for a route
    };

    /**
     * Class to manage route registration, removal and dynamic checks on request.
     */
    class RouteRegistry
    {
        /// Map holding the route key to route handler mappings.
        std::unordered_map<RouteKey, RouteHandler, RouteKeyHash> routes;

    public:
        /**
         * @brief Add new route to the registry.
         *
         * @param method Request method, i.e. GET, POST, PATCH, etc.
         * @param path Request path.
         * @param handler Request handler function.
         * @param middlewares List of @see Middleware to be imposed on this request         *
         */
        void add(const std::string& method,
                 const std::string& path,
                 RouteHandlerFunc handler,
                 const std::vector<Middleware>& middlewares);
        /**
         * @brief Add new route to the registry.
         *
         * @param method Request method, i.e. GET, POST, PATCH, etc.
         * @param path Request path.
         * @param handler Request handler function.
         * @param middlewares List of @see Middleware to be imposed on this request         *
         */
        void add(const std::string& method,
                 const std::string& path,
                 RouteHandlerFuncWithContentReader handler,
                 const std::vector<Middleware>& middlewares);
        /**
         * @brief Find a route in the registry matching given method and route.
         *
         * @param method Request method.
         * @param path Request path.
         * @return @see RouteHandler struct having middlewares and handler func.
         */
        const RouteHandler* find(const std::string& method, const std::string& path) const;

        /**
         * @brief Remove find and remove existing route + path pair from the registry
         *
         * @param method Request method
         * @param path Request path
         * @return JSON Error object, error value contains data if operation fails.
         */
        json remove(const std::string& method, const std::string& path);

        const std::string __class_name__ = "mantis::RouteRegistry";
    };

    /**
     * Class wrapper around httplib methods allowing for injection of the middleware and context functionality.
     * By default, httplib supports pre/post global middlewares, we're therefore adding a layer to scope middlewares to
     * specific routes.
     *
     * Also, to allow sharing data between middlewares and handler func, we are adding Context in the handler functions.
     */
    class HttpUnit
    {
    public:
        HttpUnit();

        void Get(const std::string& path,
                 const RouteHandlerFunc& handler,
                 std::initializer_list<Middleware> middlewares = {});

        void Post(const std::string& path,
                  const RouteHandlerFunc& handler,
                  std::initializer_list<Middleware> middlewares = {});

        void Post(const std::string& path,
                  const RouteHandlerFuncWithContentReader& handler,
                  std::initializer_list<Middleware> middlewares = {});

        void Patch(const std::string& path,
                   const RouteHandlerFunc& handler,
                   std::initializer_list<Middleware> middlewares = {});

        void Patch(const std::string& path,
                   const RouteHandlerFuncWithContentReader& handler,
                   std::initializer_list<Middleware> middlewares = {});

        void Delete(const std::string& path,
                    const RouteHandlerFunc& handler,
                    std::initializer_list<Middleware> middlewares = {});

        /**
         * @brief Bind to a port and start listening for requests.
         *
         * @param host HTTP server host.
         * @param port HTTP server port.
         * @return Flag if successful or not.
         */
        bool listen(const std::string& host, const int& port);

        /**
         * @brief Close the HTTP server connection
         */
        void close();

        static Context& context();

        /**
         * @brief Fetch the underlying route registry, check @see RouteRegistry.
         * @return Ref to the underlying route registry object.
         */
        RouteRegistry& routeRegistry();

        /**
         * @brief Get a reference to the httplib::Server instance
         *
         * @return A reference to the httplib server instance being used.
         */
        httplib::Server& server();

        const std::string _class_ = "mantis::HttpUnit";

    private:
        /**
         * @brief Decompress the response body for logging purposes, else, the logger outputs
         * gibberish data.
         *
         * @param body Response body data (compressed already if that's supported)
         * @param encoding Encoding type
         * @return Uncompressed string data
         */
        std::string decompressResponseBody(const std::string& body, const std::string& encoding);

        using Method = void (httplib::Server::*)(const std::string&, const httplib::Server::Handler&);

        template <typename HandlerType>
        using MethodBinder = std::function<void(const std::string&, HandlerType)>;

        void route(const MethodBinder<httplib::Server::Handler>& bind_method,
                   const std::string& method,
                   const std::string& path,
                   const RouteHandlerFunc& handler,
                   std::initializer_list<Middleware> middlewares);

        void route(const MethodBinder<httplib::Server::HandlerWithContentReader>& bind_method,
                   const std::string& method,
                   const std::string& path,
                   const RouteHandlerFuncWithContentReader& handler,
                   std::initializer_list<Middleware> middlewares);

        template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
        template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

        httplib::Server svr;
        RouteRegistry registry;
        thread_local static Context current_context;
    };
}

#endif // HTTPSERVER_H
