#ifndef MANTIS_SERVER_H
#define MANTIS_SERVER_H

#include <memory>
#include <vector>

namespace mantis
{
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
