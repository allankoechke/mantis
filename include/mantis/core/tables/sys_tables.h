//
// Created by allan on 20/05/2025.
//

#ifndef SYS_TABLES_H
#define SYS_TABLES_H

#include "tables.h"
#include "../../utils/utils.h"

namespace mantis
{
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
        ~SysTablesUnit() = default;

        bool setupRoutes();

        void fetchRecord(MantisRequest& req, MantisResponse& res);
        void fetchRecords(MantisRequest& req, MantisResponse& res);
        void createRecord(MantisRequest& req, MantisResponse& res, const MantisContentReader& reader);
        void updateRecord(MantisRequest& req, MantisResponse& res, const MantisContentReader& reader);
        void deleteRecord(MantisRequest& req, MantisResponse& res);

        // Auth Routes Handlers
        void authWithEmailAndPassword(MantisRequest& req, MantisResponse& res);
        bool hasAccess(MantisRequest& req, MantisResponse& res);

        json create(const json& entity, const json& opts);
        std::optional<json> read(const std::string& id, const json& opts);
        json update(const std::string& id, const json& entity, const json& opts);
        bool remove(const std::string& id, const json& opts);
        std::vector<json> list(const json& opts);

    private:
        bool itemExists(const std::string& tableName, const std::string& id) const;
    };
} // mantis

#endif //SYS_TABLES_H
