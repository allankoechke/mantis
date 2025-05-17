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

    class Router
    {
    public:
        explicit Router(MantisApp* app);
        ~Router() = default;

        bool generateCrudApis() const;

        bool startListening() const;
        void stopListening() const;

    private:
        bool generateTableCrudApis() const;
        bool generateAdminCrudApis() const;
        bool attachUserRoutes() const;

        MantisApp* m_app;
    };
}

#endif // MANTIS_SERVER_H
