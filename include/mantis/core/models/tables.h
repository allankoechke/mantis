//
// Created by allan on 13/05/2025.
//

#ifndef TABLES_H
#define TABLES_H

#include <memory>

#include "models.h"
#include "../http.h"
#include "../../app/app.h"
#include "../crud/basecrud.h"

namespace mantis
{
    /**
     *
     */
    class TableUnit: private CrudInterface<json>
    {
    public:
        explicit TableUnit(MantisApp* app,
                          const std::string& tableName,
                          const std::string& tableId,
                          const std::string& tableType = "base");

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

        json fields() const;
        void setFields(const json& fields);

        bool isSystem() const { return m_isSystem; }
        void setIsSystemTable(const bool isSystemTable);;

        // Store the rules cached
        Rule listRule();
        void setListRule(const Rule& rule);;

        Rule getRule();
        void setGetRule(const Rule& rule);;

        Rule addRule();
        void addRule(const Rule& rule);;

        Rule updateRule();
        void updateRule(const Rule& rule);;

        Rule deleteRule();
        void deleteRule(const Rule& rule);;

    private:
        json& create(const json& entity, const json& opts=json{}) override;
        std::optional<json> read(int id, const json& opts=json{}) override;
        json& update(int id, const json& entity, const json& opts=json{}) override;
        bool remove(int id, const json& opts=json{}) override;
        std::vector<json> list(const json& opts=json{}) override;


        std::unique_ptr<MantisApp> m_app;
        std::string m_tableName;
        std::string m_tableId;
        std::string m_tableType;
        std::string m_routeName;
        bool m_isSystem = false;
        std::vector<json> m_fields = {};

        // Store the rules cached
        Rule m_listRule;
        Rule m_getRule;
        Rule m_addRule;
        Rule m_updateRule;
        Rule m_deleteRule;
    };
}

#endif //TABLES_H
