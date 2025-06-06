#ifndef MANTIS_SERVER_H
#define MANTIS_SERVER_H

#include <memory>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace mantis
{
    class MantisApp;
    class TableUnit;
    class TableRoutes;

    class Router
    {
    public:
        explicit Router(MantisApp* app);
        ~Router() = default;

        bool initialize();

        bool listen() const;
        void close() const;

    private:
        bool generateTableCrudApis();
        bool generateAdminCrudApis() const;
        bool attachUserRoutes() const;

        MantisApp* m_app;
        std::shared_ptr<TableUnit> m_adminTable;
        std::shared_ptr<TableRoutes> m_tableRoutes;
        std::vector<std::shared_ptr<TableUnit>> m_routes = {};
    };
}

#endif // MANTIS_SERVER_H
