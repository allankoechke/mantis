//
// Created by allan on 18/05/2025.
//

#ifndef TABLESCRUD_H
#define TABLESCRUD_H

#include "crud.h"
#include "../database.h"
#include "../../app/app.h"

namespace mantis
{
    class MantisApp;
    class Table;

    class TablesCrud : public CrudInterface<json>
    {
    public:
        explicit TablesCrud(MantisApp* app);
        virtual ~TablesCrud() = default;

        json create(const json& entity, const json& opts) override;
        std::optional<json> read(const std::string& id, const json& opts) override;
        json update(const std::string& id, const json& entity, const json& opts) override;
        bool remove(const std::string& id, const json& opts) override;
        std::vector<json> list(const json& opts) override;

    private:
        bool itemExists(const std::string& tableName, const std::string& id) const;

        MantisApp* m_app;
    };
} // mantis

#endif //TABLESCRUD_H
