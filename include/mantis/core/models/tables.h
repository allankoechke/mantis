//
// Created by allan on 13/05/2025.
//

#ifndef TABLES_H
#define TABLES_H

#include <memory>

#include "models.h"
#include "../http.h"
#include "../../app/app.h"

namespace mantis
{
    /**
     *
     */
    class TableUnit
    {
    public:
        explicit TableUnit(MantisApp* app,
                          const std::string& tableName = "",
                          const std::string& tableId = "",
                          const std::string& tableType = "base",
                          Rule listRule = Rule{"", false},
                          Rule getRule = Rule{"", false},
                          Rule addRule = Rule{"", false},
                          Rule updateRule = Rule{"", false},
                          Rule deleteRule = Rule{"", false});

        ~TableUnit() = default;

        void setRouteDisplayName(const std::string& routeName);
        bool setupRoutes();

        void fetchRecord(const Request& req, Response& res, Context& ctx);

        void fetchRecords(const Request& req, Response& res, Context& ctx);

        void createRecord(const Request& req, Response& res, Context& ctx);

        void updateRecord(const Request& req, Response& res, Context& ctx);

        void deleteRecord(const Request& req, Response& res, Context& ctx);

        bool authWithPassword(const std::string& email, std::string& password);

        // Middleware
        static bool getAuthToken(const Request& req, Response& res, Context& ctx);
        static bool hasAccess(const Request& req, Response& res, Context& ctx);

        // Getters
        std::string tableName();
        std::string tableId();
        std::string tableType();

        // Store the rules cached
        Rule listRule();
        Rule getRule();
        Rule addRule();
        Rule updateRule();
        Rule deleteRule();

    private:
        std::unique_ptr<MantisApp> m_app;
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

#endif //TABLES_H
