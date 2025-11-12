//
// Created by codeart on 13/11/2025.
//

#include "../../../include/mantis/core/models/entity_schema_field.h"

namespace mantis {
    EntitySchemaField::EntitySchemaField(const std::string &field_name, const std::string &field_type): m_name(field_name), m_type(field_type) {
    }

    std::string EntitySchemaField::name() const { return m_name; }

    EntitySchemaField & EntitySchemaField::setName(const std::string &name) {
        if (name.empty()) throw std::invalid_argument("Field name is required!");
        m_name = name;
        return *this;
    }

    std::string EntitySchemaField::type() const { return m_type; }

    EntitySchemaField & EntitySchemaField::setType(const std::string &type) {
        if (type.empty()) throw std::invalid_argument("Field type is required!");
        // TODO check that the type is valid
        m_type = type;
        return *this;
    }

    bool EntitySchemaField::required() const { return m_required; }

    EntitySchemaField & EntitySchemaField::setRequired(const bool required) {
        m_required = required;
        return *this;
    }

    bool EntitySchemaField::isPrimaryKey() const { return m_primaryKey; }

    EntitySchemaField & EntitySchemaField::setIsPrimaryKey(const bool pk) {
        m_primaryKey = pk;
        return *this;
    }

    bool EntitySchemaField::isSystem() const { return m_isSystem; }

    EntitySchemaField & EntitySchemaField::setIsSystem(const bool system) {
        m_isSystem = system;
        return *this;
    }

    bool EntitySchemaField::isUnique() const { return m_isUnique; }

    EntitySchemaField & EntitySchemaField::setIsUnique(const bool unique) {
        m_isUnique = unique;
        return *this;
    }
} // mantis