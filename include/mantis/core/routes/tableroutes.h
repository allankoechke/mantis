//
// Created by allan on 20/05/2025.
//

#ifndef TABLEROUTES_H
#define TABLEROUTES_H

#include "../../tables/tables.h"
#include <shunting-yard.h>
#include <containers.h>

namespace mantis {
    class MantisApp;
    class TablesCrud;

class TableRoutes final : public TableUnit {
    public:
    TableRoutes(MantisApp* app,
              const std::string& tableName,
              const std::string& tableId,
              const std::string& tableType = "base");
    ~TableRoutes() override = default;

    bool setupRoutes() override;

    void fetchRecord(const Request& req, Response& res, Context& ctx) override;
    void fetchRecords(const Request& req, Response& res, Context& ctx) override;
    void createRecord(const Request& req, Response& res, Context& ctx) override;
    void updateRecord(const Request& req, Response& res, Context& ctx) override;
    void deleteRecord(const Request& req, Response& res, Context& ctx) override;

private:
    std::shared_ptr<TablesCrud> m_crud;
};


} // mantis

#endif //TABLEROUTES_H
