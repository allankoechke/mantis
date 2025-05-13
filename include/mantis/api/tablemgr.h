//
// Created by allan on 13/05/2025.
//

#ifndef TABLEMGR_H
#define TABLEMGR_H

#include "httpserver.h"
#include <memory>

#include "mantis/core/models.h"

namespace Mantis
{
    class MantisApp;

    class TableMgr {
    public:
        explicit TableMgr(const MantisApp& app);
        ~TableMgr() = default;

        void FetchRecord(const Request& req, Response& res, Context& ctx);

        void FetchRecords(const Request& req, Response& res, Context& ctx);

        void CreateRecord(const Request& req, Response& res, Context& ctx);

        void UpdateRecord(const Request& req, Response& res, Context& ctx);

        void DeleteRecord(const Request& req, Response& res, Context& ctx);

        bool AuthWithPassword(const std::string& email, std::string& password);

    private:
        std::shared_ptr<MantisApp> m_app;
        std::string m_tableName;
        std::string m_tableId;

        // Store the rules cached
        Rule m_listRule;
        Rule m_getRule;
        Rule m_addRule;
        Rule m_updateRule;
        Rule m_deleteRule;
    };
}

#endif //TABLEMGR_H
