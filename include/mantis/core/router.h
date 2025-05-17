#ifndef MANTIS_SERVER_H
#define MANTIS_SERVER_H

#include <string>
#include <memory>
#include <soci/soci.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mantis
{
    class MantisApp;
    class TableUnit;

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
        bool generateAdminCrudApis();
        bool attachUserRoutes() const;

        MantisApp* m_app;
        std::vector<std::shared_ptr<TableUnit>> m_routes = {};
    };
}

#endif // MANTIS_SERVER_H
