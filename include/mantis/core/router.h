/**
 * @brief router.h
 * @brief Router file for higher level route registration and removal *
 */

#ifndef MANTIS_SERVER_H
#define MANTIS_SERVER_H

#include <memory>
#include <vector>
#include <nlohmann/json.hpp>

namespace mantis {
    using json = nlohmann::json;

    class TableUnit;
    class SysTablesUnit;

    /**
     * @brief Router class allows for managing routes as well as acting as a top-wrapper on the HttpUnit.
     */
    class Router {
    public:
        explicit Router();

        ~Router() = default;

        /// Initialize the router instance creating tables and admin routes.
        bool initialize();

        /// Bind to a port and start listening for new connections.
        /// Calls @see HttpUnit::listen() under the hood.
        bool listen() const;

        /// Close the HTTP Server connection
        /// Calls @see HttpUnit::close()
        void close();

        /// Restart the HTTP server
        // void restart();

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

        const std::string __class_name__ = "mantis::Router";

    private:
        /**
         * @brief Generate API endpoint for file serving for record level file types.
         * @return Status whether API generation succeeded.
         */
        bool generateFileServingApi() const;

        /**
         * @brief Generate CRUD API endpoints for all managed table schema(s).
         * @return Status whether CRUD APIs generation succeeded
         */
        bool generateTableCrudApis();

        bool generateMiscEndpoints() const;

        /**
         * @brief Generate Admin only CRUD endpoints.
         * @return Status whether Admin only CRUD  generation succeeded
         */
        bool generateAdminCrudApis() const;

        std::shared_ptr<TableUnit> m_adminTable;
        std::shared_ptr<SysTablesUnit> m_tableRoutes;
        std::vector<std::shared_ptr<TableUnit> > m_routes = {};

    public:
        std::vector<json> adminTableFields = {};
    };
}

#endif // MANTIS_SERVER_H
