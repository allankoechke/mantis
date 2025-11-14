//
// Created by codeart on 13/11/2025.
//

#include "../../../include/mantis/core/models/entity_schema_field.h"

namespace mantis {
    std::vector<std::string> EntitySchemaField::defaultBaseFields = {"id", "created", "updated"};
    std::vector<std::string> EntitySchemaField::defaultAuthFields = {
        "id", "created", "updated", "name", "email", "password"
    };
    std::vector<std::string> EntitySchemaField::defaultEntityFieldTypes = {
        "xml", "string", "double", "date", "int8", "uint8", "int16", "uint16", "int32", "uint32", "int64", "uint64",
        "blob", "json", "bool", "file", "files"
    };

    EntitySchemaField::EntitySchemaField(const std::string &field_name, const std::string &field_type)
        : m_name(field_name), m_type(field_type) {
    }

    EntitySchemaField::EntitySchemaField(const nlohmann::json &field) {
        // nlohmann::json m_constraints; // "min-value", "max_value", "validator", "default_value"
        if (!field.contains("name") || field["name"].is_null())
            throw std::invalid_argument("Field name is required!");
        setName(field["name"].get<std::string>());

        if (!field.contains("type") || field["type"].is_null())
            throw std::invalid_argument("Field type is required!");
        setType(field["type"].get<std::string>());

        if (field.contains("required") && field["required"].is_boolean())
            setRequired(field["required"].get<bool>());

        if (field.contains("primary_key") && field["primary_key"].is_boolean())
            setIsPrimaryKey(field["primary_key"].get<bool>());

        if (field.contains("system") && field["system"].is_boolean())
            setIsSystem(field["system"].get<bool>());

        if (field.contains("unique") && field["unique"].is_boolean())
            setRequired(field["unique"].get<bool>());

        if (field.contains("constraints") && field["constraints"].is_object())
            setConstraints(field["constraints"].get<nlohmann::json>());
    }

    std::string EntitySchemaField::name() const { return m_name; }

    EntitySchemaField &EntitySchemaField::setName(const std::string &name) {
        if (name.empty()) throw std::invalid_argument("Field name is required!");
        m_name = name;
        return *this;
    }

    std::string EntitySchemaField::type() const { return m_type; }

    EntitySchemaField &EntitySchemaField::setType(const std::string &type) {
        if (type.empty()) throw std::invalid_argument("Field type is required!");
        // TODO check that the type is valid
        m_type = type;
        return *this;
    }

    bool EntitySchemaField::required() const { return m_required; }

    EntitySchemaField &EntitySchemaField::setRequired(const bool required) {
        m_required = required;
        return *this;
    }

    bool EntitySchemaField::isPrimaryKey() const { return m_primaryKey; }

    EntitySchemaField &EntitySchemaField::setIsPrimaryKey(const bool pk) {
        m_primaryKey = pk;
        return *this;
    }

    bool EntitySchemaField::isSystem() const { return m_isSystem; }

    EntitySchemaField &EntitySchemaField::setIsSystem(const bool system) {
        m_isSystem = system;
        return *this;
    }

    bool EntitySchemaField::isUnique() const { return m_isUnique; }

    EntitySchemaField &EntitySchemaField::setIsUnique(const bool unique) {
        m_isUnique = unique;
        return *this;
    }

    nlohmann::json EntitySchemaField::constraints() const { return m_constraints; }

    nlohmann::json EntitySchemaField::constraints(const std::string &key) const {
        return m_constraints.contains(key) ? m_constraints[key] : nullptr;
    }

    EntitySchemaField &EntitySchemaField::setConstraints(const nlohmann::json &opts) {
        if (m_constraints.is_null())
            m_constraints = opts;

        nlohmann::json constraints = nlohmann::json::object();
        if (opts.contains("validator"))
            constraints["validator"] = opts["validator"];

        if (opts.contains("default_value"))
            constraints["default_value"] = opts["default_value"];

        if (opts.contains("min_value") && opts["min_value"].is_number())
            constraints["min_value"] = opts["min_value"];

        if (opts.contains("max_value") && opts["max_value"].is_number())
            constraints["max_value"] = opts["max_value"];

        m_constraints = constraints;
        return *this;
    }

    nlohmann::json EntitySchemaField::toJson() const {
        return {
            {"name", m_name},
            {"type", m_type},
            {"required", m_required},
            {"primary_key", m_primaryKey},
            {"system", m_isSystem},
            {"unique", m_isUnique},
            {"constraints", m_constraints}
        };
    }

    soci::db_type EntitySchemaField::toSociType() const {
        return EntitySchemaField::toSociType(m_type);
    }

    soci::db_type EntitySchemaField::toSociType(const std::string &type) {
        if (type == "json")
            return soci::db_string;
        if (type == "xml")
            return soci::db_xml;
        if (type == "double")
            return soci::db_double;
        if (type == "date")
            return soci::db_date;
        if (type == "int8")
            return soci::db_int8;
        if (type == "uint8")
            return soci::db_uint8;
        if (type == "int16")
            return soci::db_int16;
        if (type == "uint16")
            return soci::db_uint16;
        if (type == "int32")
            return soci::db_int32;
        if (type == "uint32")
            return soci::db_uint32;
        if (type == "int64")
            return soci::db_int64;
        if (type == "uint64")
            return soci::db_uint64;
        if (type == "blob")
            return soci::db_blob;
        if (type == "bool")
            return soci::db_uint16;
        if (type == "string" || type == "file" || type == "files")
            return soci::db_string;

        throw std::invalid_argument("Unknown field type");
    }

    bool EntitySchemaField::isValidFieldType(const std::string &type) {
        const auto it = std::ranges::find(defaultEntityFieldTypes, type);
        return it != defaultEntityFieldTypes.end();
    }
} // mantis
