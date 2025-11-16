/**
 * @brief router.h
 * @brief Router file for higher level route registration and removal *
 */

#ifndef MANTIS_SERVER_H
#define MANTIS_SERVER_H

#include <memory>
#include <vector>
#include <nlohmann/json.hpp>
#include <httplib.h>

#include "route_registry.h"
#include "../utils/utils.h"
#include "models/entity.h"

namespace mantis {
    class MantisRequest;
    class MantisResponse;
    using json = nlohmann::json;

    /**
     * @brief Router class allows for managing routes as well as acting as a top-wrapper on the HttpUnit.
     */
    class Router {
    public:
        Router();

        ~Router();

        /// Initialize the router instance creating tables and admin routes.
        bool initialize();

        /// Bind to a port and start listening for new connections.
        /// Calls @see HttpUnit::listen() under the hood.
        bool listen();

        /// Close the HTTP Server connection
        /// Calls @see HttpUnit::close()
        void close();

        httplib::Server& server();

        // ----------- HTTP METHODS ----------- //
        void Get(const std::string &path, HandlerFn handler, Middlewares middlewares = {});

        void Post(const std::string &path, HandlerFn handler, Middlewares middlewares = {});

        void Patch(const std::string &path, HandlerFn handler, Middlewares middlewares = {});

        void Delete(const std::string &path, HandlerFn handler, Middlewares middlewares = {});

        // Manage routes
        [[nodiscard]]
        /// Adds a new table route passed on a given table name. Used mostly when a table is added after the server is
        /// already running. The table has to be existing for routes to be created for.
        /// @param table Table name
        /// @return JSON object having `success` and `error` values.
        json addRoute(const std::string &table);

        /// Update existing route, probably due to a change in table name and/or table type
        /// @param table_data Contains the changes, that is, `old_type`, `old_name` and `new_name` matching the old
        /// table name and type together with the new table name. The new table type will be fetched from the db.
        /// @return JSON object having `success` and `error` values.
        json updateRoute(const json &table_data = json::object());

        /// Update existing route internal data, probably due to a change in table internal metadata
        /// @param table_data Contains the new table schema
        /// @return JSON object having `success` and `error` values.
        json updateRouteCache(const json &table_data = json::object());

        /// Remove existing route(s), maybe due to deletion of a table.
        /// @param table_data must have the deleted table's `name` and `type`.
        /// @return JSON object having `success` and `error` values.
        json removeRoute(const json &table_data = json::object());

        const json &schemaCache(const std::string &table_name) const;

        void addSchemaCache(const std::string &table_name, const nlohmann::json &table_schema);

        void updateSchemaCache(const std::string &table_name, const json &table_schema);

        void removeSchemaCache(const std::string &table_name);

    private:
        void globalRouteHandler(const std::string &method, const std::string &path);

        void generateMiscEndpoints() const;

        static std::string getMimeType(const std::string &path);;

        httplib::Server svr;
        RouteRegistry m_routeRegistry;
        std::vector<MiddlewareFn> m_globalMiddlewares;
        std::vector<nlohmann::json> m_schemas;
        std::unordered_map<std::string, Entity> m_entityMap;
    };
}

#endif // MANTIS_SERVER_H
