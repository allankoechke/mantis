//
// Created by allan on 13/05/2025.
//

#ifndef TABLEMGR_H
#define TABLEMGR_H

#include "../http.h"
#include <memory>

#include "mantis/mantis.h"
#include "./models.h"

namespace mantis
{
    class Router;

    class TableMgr
    {
    public:
        explicit TableMgr(const Router& svrMgr,
                          const std::string& tableName = "",
                          const std::string& tableId = "",
                          const std::string& tableType = "base",
                          Rule listRule = Rule{"", false},
                          Rule getRule = Rule{"", false},
                          Rule addRule = Rule{"", false},
                          Rule updateRule = Rule{"", false},
                          Rule deleteRule = Rule{"", false});

        ~TableMgr() = default;

        void SetRouteDisplayName(const std::string& routeName);
        bool SetupRoutes();

        void FetchRecord(const Request& req, Response& res, Context& ctx);

        void FetchRecords(const Request& req, Response& res, Context& ctx);

        void CreateRecord(const Request& req, Response& res, Context& ctx);

        void UpdateRecord(const Request& req, Response& res, Context& ctx);

        void DeleteRecord(const Request& req, Response& res, Context& ctx);

        bool AuthWithPassword(const std::string& email, std::string& password);

        // Middleware
        static bool HasAuthHeader(const Request& req, Response& res, Context& ctx);
        static bool HasAccessPermission(const Request& req, Response& res, Context& ctx);

        // Getters
        std::string tableName();
        std::string tableId();
        std::string tableType()
        {
            return m_tableType;
        }

        // Store the rules cached
        Rule listRule();
        Rule getRule();
        Rule addRule();
        Rule updateRule();
        Rule deleteRule();

    private:
        std::shared_ptr<Router> m_svrMgr;
        std::string m_tableName;
        std::string m_tableId;
        std::string m_tableType;
        std::string m_routeName;

        // Store the rules cached
        Rule m_listRule;
        Rule m_getRule;
        Rule m_addRule;
        Rule m_updateRule;
        Rule m_deleteRule;
    };
}

#endif //TABLEMGR_H
