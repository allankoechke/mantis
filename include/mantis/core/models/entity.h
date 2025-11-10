//
// Created by codeart on 09/11/2025.
//

#ifndef MANTISAPP_ENTITY_H
#define MANTISAPP_ENTITY_H

#include <string>
#include "mantis/mantis.h"
#include "mantis/utils/soci_wrappers.h"

namespace mantis {
    using Record = json;
    using Records = std::vector<Record>;

    class Entity {
    public:
        /**
         * @brief `Entity`
         * @param schema Table schema
         */
        explicit Entity(const json &schema);

        // --------------- DB TABLE OPS ------------------ //
        [[nodiscard]] std::string id() const;

        [[nodiscard]] const std::string &name() const;

        [[nodiscard]] const std::string &type() const;

        [[nodiscard]] bool isSystem() const;

        [[nodiscard]] bool hasApi() const;

        [[nodiscard]] const std::vector<json> &fields() const;

        [[nodiscard]] const std::string &listRule() const;

        [[nodiscard]] const std::string &getRule() const;

        [[nodiscard]] const std::string &addRule() const;

        [[nodiscard]] const std::string &updateRule() const;

        [[nodiscard]] const std::string &deleteRule() const;

        // --------------- DB CRUD OPS ------------------ //
        [[nodiscard]] Record create(const ::json &Record, const json &opts = json::object()) const;

        [[nodiscard]] Records list(const json &opts = json::object()) const;

        [[nodiscard]] std::optional<Record> read(const std::string &id, const json &opts = json::object()) const;

        [[nodiscard]] Record update(const std::string &id, const json &data, const json &opts = json::object()) const;

        void remove(const std::string &id) const;

        // --------------- SCHEMA OPS ------------------ //
        [[nodiscard]] const json &schema() const;

        // --------------- UTILITY OPS ------------------ //
        [[nodiscard]] bool recordExists(const std::string &id) const;

        [[nodiscard]] std::optional<json> findField(const std::string &field_name) const;

        [[nodiscard]] std::optional<json> queryFromCols(const std::string &value, const std::vector<std::string> &columns) const;

    private:
        json m_schema;
    };
} // mantis

#endif //MANTISAPP_ENTITY_H
