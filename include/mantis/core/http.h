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

#ifdef MANTIS_ENABLE_SCRIPTING
#include <dukglue/dukglue.h>
#endif

#include "logging.h"
#include "mantis/app/app.h"
#include "private-impl/duktape_custom_types.h"
#include "../utils/utils.h"

#define REQUEST_HANDLED false
#define REQUEST_PENDING true

namespace mantis
{
    class MantisApp;

    /// Shorten JSON namespace
    using json = nlohmann::json;

    ///> Middleware shorthand for the content reader
    using MantisContentReader = httplib::ContentReader;

    ///> Middleware shorthand for the function
    using MiddlewareFunc = std::function<bool(MantisRequest&, MantisResponse&)>;

    ///> Route Handler function shorthand
    using RouteHandlerFunc = std::function<void(MantisRequest&, MantisResponse&)>;

    ///> Route Handler function with content reader shorthand
    using RouteHandlerFuncWithContentReader = std::function<void(MantisRequest&, MantisResponse&,
                                                                 const MantisContentReader&)>;

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
        std::vector<MiddlewareFunc> middlewares; ///> List of @see Middlewares for a route.
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
                 const std::vector<MiddlewareFunc>& middlewares);
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
                 const std::vector<MiddlewareFunc>& middlewares);
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
        ~HttpUnit();

        void Get(const std::string& path,
                 const RouteHandlerFunc& handler,
                 std::initializer_list<MiddlewareFunc> middlewares = {});

        void Post(const std::string& path,
                  const RouteHandlerFunc& handler,
                  std::initializer_list<MiddlewareFunc> middlewares = {});

        void Post(const std::string& path,
                  const RouteHandlerFuncWithContentReader& handler,
                  std::initializer_list<MiddlewareFunc> middlewares = {});

        void Patch(const std::string& path,
                   const RouteHandlerFunc& handler,
                   std::initializer_list<MiddlewareFunc> middlewares = {});

        void Patch(const std::string& path,
                   const RouteHandlerFuncWithContentReader& handler,
                   std::initializer_list<MiddlewareFunc> middlewares = {});

        void Delete(const std::string& path,
                    const RouteHandlerFunc& handler,
                    std::initializer_list<MiddlewareFunc> middlewares = {});

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

        /**
         * @brief Generate hash for the file metadata
         * @param data Multipart file reference
         * @return Hash
         */
        static std::string hashMultipartMetadata(const httplib::FormData& data);


#ifdef MANTIS_ENABLE_SCRIPTING
        static void registerDuktapeMethods();
#endif


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
        static std::string decompressResponseBody(const std::string& body, const std::string& encoding);

        template <typename HandlerType>
        using MethodBinder = std::function<void(const std::string&, HandlerType)>;

        void route(const MethodBinder<httplib::Server::Handler>& bind_method,
                   const std::string& method,
                   const std::string& path,
                   const RouteHandlerFunc& handler,
                   std::initializer_list<MiddlewareFunc> middlewares);

        void route(const MethodBinder<httplib::Server::HandlerWithContentReader>& bind_method,
                   const std::string& method,
                   const std::string& path,
                   const RouteHandlerFuncWithContentReader& handler,
                   std::initializer_list<MiddlewareFunc> middlewares);

        httplib::Server svr;
        RouteRegistry registry;
    };
}

#endif // HTTPSERVER_H
