//
// Created by allan on 13/05/2025.
//

#ifndef ADMINMGR_H
#define ADMINMGR_H

#include <mantis/mantis.h>
#include "httpserver.h"

namespace Mantis
{
    class AdminMgr {
    public:
        AdminMgr(const MantisApp& app) : m_app(make_shared<MantisApp>(app)) {}
        ~AdminMgr() = default;

        void FetchAdminRecord(const Request& req, Response& res, Context& ctx);

        void FetchAdminRecords(const Request& req, Response& res, Context& ctx);

        void CreateAdminRecord(const Request& req, Response& res, Context& ctx);

        void UpdateAdminRecord(const Request& req, Response& res, Context& ctx);

        void DeleteAdminRecord(const Request& req, Response& res, Context& ctx);

        bool AuthWithPassword(const std::string& email, std::string& password);

    private:
        shared_ptr<MantisApp> m_app;
    };
}

#endif //ADMINMGR_H
