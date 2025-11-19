//
// Created by codeart on 09/11/2025.
//

#ifndef MANTISAPP_ENTITY_H
#define MANTISAPP_ENTITY_H

#include <string>
#include "mantisbase/mantis.h"
#include "mantisbase/core/exceptions.h"
#include "mantisbase/utils/soci_wrappers.h"

namespace mantis {
    using Record = nlohmann::json;
    using Records = std::vector<Record>;

    class Entity {
    public:
        /**
         * @brief `Entity`
         * @param schema Table schema
         */
        explicit Entity(const nlohmann::json &schema);

        // --------------- DB TABLE OPS ------------------ //
        [[nodiscard]] std::string id() const;

        [[nodiscard]] std::string name() const;

        [[nodiscard]] std::string type() const;

        [[nodiscard]] bool isSystem() const;

        [[nodiscard]] bool hasApi() const;

        [[nodiscard]] const std::vector<json> &fields() const;

        [[nodiscard]] std::string listRule() const;

        [[nodiscard]] std::string getRule() const;

        [[nodiscard]] std::string addRule() const;

        [[nodiscard]] std::string updateRule() const;

        [[nodiscard]] std::string deleteRule() const;

        // --------------- DB CRUD OPS ------------------ //
        [[nodiscard]] Record create(const json &Record, const json &opts = json::object()) const;

        [[nodiscard]] Records list(const json &opts = json::object()) const;

        [[nodiscard]] std::optional<Record> read(const std::string &id, const json &opts = json::object()) const;

        [[nodiscard]] Record update(const std::string &id, const json &data, const json &opts = json::object()) const;

        void remove(const std::string &id) const;

        // --------------- SCHEMA OPS ------------------ //
        [[nodiscard]] const json &schema() const;

        // --------------- UTILITY OPS ------------------ //
        [[nodiscard]] bool recordExists(const std::string &id) const;

        [[nodiscard]] std::optional<json> findField(const std::string &field_name) const;

        [[nodiscard]] std::optional<json> queryFromCols(const std::string &value,
                                                        const std::vector<std::string> &columns) const;

        // --------------- SCHEMA ROUTING ------------------ //
        [[nodiscard]] HandlerFn getOneRouteHandler() const;

        [[nodiscard]] HandlerFn getManyRouteHandler() const;

        [[nodiscard]] HandlerFn postRouteHandler() const;

        [[nodiscard]] HandlerFn patchRouteHandler() const;

        [[nodiscard]] HandlerFn deleteRouteHandler() const;

        [[nodiscard]] HandlerFn authRouteHandler() const;

        void createEntityRoutes() const;

    private:
        nlohmann::json m_schema;
    };
} // mantis

#endif //MANTISAPP_ENTITY_H
