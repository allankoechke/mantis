//
// Created by allan on 20/05/2025.
//

#ifndef SYS_TABLES_H
#define SYS_TABLES_H

#include "tables.h"


namespace mantis
{
#define DUMP_RESPONSE() Log::trace("DUMP_RESPONSE: {}", response.dump())

    class TablesCrud;

    /**
     * @brief Class to manage tables metadata like access rules, name, etc.
     *
     * Allows for table tracking, provisioning of API access and managing cache for created tables
     * for centralized admin management.
     */
    class SysTablesUnit final : public TableUnit
    {
    public:
        SysTablesUnit(const std::string& tableName,
                      const std::string& tableId,
                      const std::string& tableType = "auth");
        ~SysTablesUnit() override = default;

        bool setupRoutes() override;

        void fetchRecord(const Request& req, Response& res, Context& ctx) override;
        void fetchRecords(const Request& req, Response& res, Context& ctx) override;
        void createRecord(const Request& req, Response& res, const ContentReader& reader, Context& ctx) override;
        void updateRecord(const Request& req, Response& res, const ContentReader& reader, Context& ctx) override;
        void deleteRecord(const Request& req, Response& res, Context& ctx) override;

        // Auth Routes Handlers
        void authWithEmailAndPassword(const Request& req, Response& res, Context& ctx) override;
        bool hasAccess(const Request& req, Response& res, Context& ctx) override;

        json create(const json& entity, const json& opts) override;
        std::optional<json> read(const std::string& id, const json& opts) override;
        json update(const std::string& id, const json& entity, const json& opts) override;
        bool remove(const std::string& id, const json& opts) override;
        std::vector<json> list(const json& opts) override;

    private:
        bool itemExists(const std::string& tableName, const std::string& id) const;
    };
} // mantis

#endif //SYS_TABLES_H
