//
// Created by allan on 13/05/2025.
//

#ifndef TABLES_H
#define TABLES_H

#include <memory>
#include <nlohmann/json.hpp>

#include "../models/models.h"
#include "../http.h"
#include "../crud/crud.h"
#include "../jwt.h"
#include "../../app/app.h"

namespace mantis
{
    using json = nlohmann::json;

    // using TableValue = std::variant<
    //     std::monostate,
    //     std::string,
    //     double,
    //     std::tm,
    //     int8_t, uint8_t,
    //     int16_t, uint16_t,
    //     int32_t, uint32_t,
    //     int64_t, uint64_t,
    //     bool,
    //     json,
    //     BLOB
    // >;

    /**
     *
     */
    class TableUnit : public CrudInterface<json>
    {
    public:
        explicit TableUnit(std::string tableName,
                           std::string tableId,
                           std::string tableType = "base");
        explicit TableUnit(const json& schema = json::object());

        virtual ~TableUnit() override = default;

        // CRUD endpoints handlers
        virtual void fetchRecord(const Request& req, Response& res, Context& ctx);
        virtual void fetchRecords(const Request& req, Response& res, Context& ctx);
        virtual void createRecord(const Request& req, Response& res, Context& ctx);
        virtual void updateRecord(const Request& req, Response& res, Context& ctx);
        virtual void deleteRecord(const Request& req, Response& res, Context& ctx);

        // Auth Routes Handlers
        virtual void authWithEmailAndPassword(const Request& req, Response& res, Context& ctx);
        virtual void resetPassword(const Request& req, Response& res, Context& ctx);

        // Router setup
        virtual bool setupRoutes();

        // Override route display name
        void setRouteDisplayName(const std::string& routeName);

        // Middleware
        static bool getAuthToken(const Request& req, Response& res, Context& ctx);
        virtual bool hasAccess(const Request& req, Response& res, Context& ctx);

        // Getters
        std::string tableName();
        void setTableName(const std::string& name);

        std::string tableId();
        void setTableId(const std::string& id);

        std::string tableType();
        void fromJson(const json& j);

        std::vector<json> fields() const;
        void setFields(const std::vector<json>& fields);

        bool isSystem() const;
        void setIsSystemTable(bool isSystemTable);

        // Store the rules cached
        Rule listRule();
        void setListRule(const Rule& rule);

        Rule getRule();
        void setGetRule(const Rule& rule);

        Rule addRule();
        void setAddRule(const Rule& rule);

        Rule updateRule();
        void setUpdateRule(const Rule& rule);

        Rule deleteRule();
        void setDeleteRule(const Rule& rule);

        // CRUD endpoints
        // Create/read/list/update/delete record(s), use opts to config optional params
        json create(const json& entity, const json& opts) override;
        std::optional<json> read(const std::string& id, const json& opts) override;
        json update(const std::string& id, const json& entity, const json& opts) override;
        bool remove(const std::string& id, const json& opts) override;
        std::vector<json> list(const json& opts) override;

        // Helper methods
        static std::string generateTableId(const std::string& tablename);
        std::string getColTypeFromName(const std::string& col, const std::vector<json>& fields) const;
        json parseDbRowToJson(const soci::row& row) const;
        std::optional<json> validateRequestBody(const json& body) const;
        std::optional<json> validateUpdateRequestBody(const json& body) const;
        bool recordExists(const std::string& id) const;
        std::optional<json> findFieldByKey(const std::string& key) const;
        json checkValueInColumns(const std::string& value, const std::vector<std::string>& columns) const;

    protected:
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
