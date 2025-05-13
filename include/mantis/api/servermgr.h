#ifndef MANTIS_SERVER_H
#define MANTIS_SERVER_H

#include <string>
#include <memory>
#include "httpserver.h"

namespace Mantis
{
    class MantisApp;

    class ServerMgr
    {
    public:
        explicit ServerMgr(const MantisApp& app);
        ~ServerMgr() = default;

        bool GenerateCrudApis();

        bool StartListening() const;
        bool StopListening() const;

        std::string Host() const;
        void SetHost(const std::string& host);

        int Port() const;
        void SetPort(const int& port);

    private:
        bool GenerateTableCrudApis();
        bool GenerateAdminCrudApis();
        bool AttachUserRoutes();

        // ---
        // Member Variables Section
        std::shared_ptr<MantisApp> m_app;
        std::shared_ptr<HttpServer> m_httpServer;

        int m_port = 7070;
        std::string m_host = "127.0.0.1";

        /// Rules & Schema Cache
    };
}

#endif // MANTIS_SERVER_H
