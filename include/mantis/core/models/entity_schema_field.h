//
// Created by codeart on 13/11/2025.
//

#ifndef MANTISBASE_ENTITY_SCHEMA_FIELD_H
#define MANTISBASE_ENTITY_SCHEMA_FIELD_H

#include <optional>
#include <string>

#include "nlohmann/json.hpp"
#include "soci/soci-backend.h"

namespace mantis {
    class EntitySchemaField {
        std::string m_name;
        std::string m_type;
        bool m_required = false, m_primaryKey = false, m_isSystem = false, m_isUnique = false;
        nlohmann::json m_constraints; // "min-value", "max_value", "validator", "default_value"

    public:
        static std::vector<std::string> defaultBaseFields;
        static std::vector<std::string> defaultAuthFields;
        static std::vector<std::string> defaultEntityFieldTypes;

        // Convenience constructor
        EntitySchemaField(const std::string &field_name, const std::string &field_type);

        EntitySchemaField(const nlohmann::json &field);

        std::string name() const;

        EntitySchemaField &setName(const std::string &name);

        std::string type() const;

        EntitySchemaField &setType(const std::string &type);

        bool required() const;

        EntitySchemaField &setRequired(bool required);

        bool isPrimaryKey() const;

        EntitySchemaField &setIsPrimaryKey(bool pk);

        bool isSystem() const;

        EntitySchemaField &setIsSystem(bool system);

        bool isUnique() const;

        EntitySchemaField &setIsUnique(const bool unique);

        nlohmann::json constraints() const;

        nlohmann::json constraints(const std::string& key) const;

        EntitySchemaField &setConstraints(const nlohmann::json &opts);

        [[nodiscard]] nlohmann::json toJson() const;

        [[nodiscard]] soci::db_type toSociType() const;

        [[nodiscard]] static soci::db_type toSociType(const std::string &type);

        static bool isValidFieldType(const std::string &type);
    };
} // mantis

#endif //MANTISBASE_ENTITY_SCHEMA_FIELD_H
