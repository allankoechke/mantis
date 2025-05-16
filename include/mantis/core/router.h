#ifndef MANTIS_SERVER_H
#define MANTIS_SERVER_H

#include <string>
#include <memory>
#include "http.h"

namespace mantis
{
    class MantisApp;

    class Router
    {
    public:
        explicit Router(const MantisApp& app);
        ~Router() = default;

        bool generateCrudApis() const;

        bool startListening() const;
        bool stopListening() const;

        std::string host() const;
        void setHost(const std::string& host);

        int port() const;
        void setPort(const int& port);

    private:
        bool generateTableCrudApis() const;
        bool generateAdminCrudApis() const;
        bool attachUserRoutes() const;

        // Member Variables Section
        MantisApp* m_app;

        /// Rules & Schema Cache
    };
}

#endif // MANTIS_SERVER_H
