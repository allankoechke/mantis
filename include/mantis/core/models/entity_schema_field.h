//
// Created by codeart on 13/11/2025.
//

#ifndef MANTISBASE_ENTITY_SCHEMA_FIELD_H
#define MANTISBASE_ENTITY_SCHEMA_FIELD_H

#include <optional>
#include <string>

#include "nlohmann/json.hpp"

namespace mantis {
    class EntitySchemaField {
        std::string m_name;
        std::string m_type;

        bool m_required = false, m_primaryKey = false, m_isSystem = false, m_isUnique = false;

        std::optional<std::string> defaultValue; // as string, parse based on type
        std::optional<std::string> regexPattern;
        std::optional<double> minValue;
        std::optional<double> maxValue;
        std::optional<std::string> validator;
        std::optional<std::string> autoGeneratePattern; // regex for auto-gen strings

    public:
        // Convenience constructor
        EntitySchemaField(const std::string &field_name, const std::string &field_type);

        std::string name() const;

        EntitySchemaField& setName(const std::string& name);

        std::string type() const;

        EntitySchemaField& setType(const std::string& type);

        bool required() const;

        EntitySchemaField& setRequired(const bool required);

        bool isPrimaryKey() const;

        EntitySchemaField& setIsPrimaryKey(const bool pk);

        bool isSystem() const;

        EntitySchemaField& setIsSystem(const bool system);

        bool isUnique() const;

        EntitySchemaField& setIsUnique(const bool unique);

        // [[nodiscard]]
        // nlohmann::json to_json() const;
        //
        // [[nodiscard]]
        // soci::db_type toSociType() const;
        //
        // [[nodiscard]]
        // static soci::db_type toSociType(const FieldType &f_type);
    };
} // mantis

#endif //MANTISBASE_ENTITY_SCHEMA_FIELD_H
