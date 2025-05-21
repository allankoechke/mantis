//
// Created by allan on 20/05/2025.
//

#include "../../../include/mantis/core/routes/tableroutes.h"
#include "../../../include/mantis/app/app.h"

namespace mantis {
    TableRoutes::TableRoutes(MantisApp* app, const std::string& tableName, const std::string& tableId,
        const std::string& tableType): TableUnit(app, tableName, tableId, tableType)
    {}

    bool TableRoutes::setupRoutes()
    {
        return true;
    }

    void TableRoutes::fetchRecord(const Request& req, Response& res, Context& ctx)
    {}

    void TableRoutes::fetchRecords(const Request& req, Response& res, Context& ctx)
    {}

    void TableRoutes::createRecord(const Request& req, Response& res, Context& ctx)
    {}

    void TableRoutes::updateRecord(const Request& req, Response& res, Context& ctx)
    {}

    void TableRoutes::deleteRecord(const Request& req, Response& res, Context& ctx)
    {}
} // mantis