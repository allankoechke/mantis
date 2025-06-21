#ifndef MANTIS_SERVER_H
#define MANTIS_SERVER_H

#include <memory>
#include <vector>
#include <nlohmann/json.hpp>

namespace mantis
{
    using json = nlohmann::json;

    class TableUnit;
    class SysTablesUnit;

    class Router
    {
    public:
        explicit Router();
        ~Router() = default;

        bool initialize();

        bool listen() const;
        void close();
        void restart();

        // Manage routes
        [[nodiscard]]
        json addRoute(const std::string& table);
        json updateRoute(const json& table_data = json::object());
        json removeRoute(const json& table_data = json::object());

    private:
        bool generateTableCrudApis();
        bool generateAdminCrudApis() const;
        bool attachUserRoutes() const;

        std::shared_ptr<TableUnit> m_adminTable;
        std::shared_ptr<SysTablesUnit> m_tableRoutes;
        std::vector<std::shared_ptr<TableUnit>> m_routes = {};
    };
}

#endif // MANTIS_SERVER_H
