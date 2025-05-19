//
// Created by allan on 18/05/2025.
//

#ifndef TABLESCRUD_H
#define TABLESCRUD_H

#include "crud.h"

namespace mantis {
    class MantisApp;
    class Table;

class TablesCrud: public CrudInterface<json> {
    public:
    TablesCrud(MantisApp* app, Table* t);
    virtual ~TablesCrud() = default;

    json& create(const json& entity, const json& opts=json{}) override;

    std::optional<json> read(const std::string& id, const json& opts=json{}) override;

    json& update(const std::string& id, const json& entity, const json& opts=json{}) override;

    bool remove(const std::string& id, const json& opts=json{}) override;

    std::vector<json> list(const json& opts=json{}) override;

private:
    MantisApp* m_app;
    Table* m_table;
};
} // mantis

#endif //TABLESCRUD_H
