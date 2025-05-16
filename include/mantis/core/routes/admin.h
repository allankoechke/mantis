//
// Created by allan on 13/05/2025.
//

#ifndef ADMINMGR_H
#define ADMINMGR_H

#include "../http.h"
#include "../../app/app.h"

namespace mantis
{
    class AdminCrudUser {
    public:
        AdminCrudUser(MantisApp* app) : m_app(app) {}
        ~AdminCrudUser() = default;

        void FetchAdminRecord(const Request& req, Response& res, Context& ctx);

        void FetchAdminRecords(const Request& req, Response& res, Context& ctx);

        void CreateAdminRecord(const Request& req, Response& res, Context& ctx);

        void UpdateAdminRecord(const Request& req, Response& res, Context& ctx);

        void DeleteAdminRecord(const Request& req, Response& res, Context& ctx);

        bool AuthWithPassword(const std::string& email, std::string& password);

    private:
        std::unique_ptr<MantisApp> m_app;
    };
}

#endif //ADMINMGR_H
