//
// Created by codeart on 12/11/2025.
//

#include "../../../include/mantis/core/models/entity_schema.h"

namespace mantis {
    EntitySchema::EntitySchema() = default;

    EntitySchema::EntitySchema(const EntitySchema &) = default;

    EntitySchema & EntitySchema::operator=(const EntitySchema &) = default;

    EntitySchema::EntitySchema(EntitySchema &&) = default;

    EntitySchema & EntitySchema::operator=(EntitySchema &&) = default;

    EntitySchema::~EntitySchema() = default;

    EntitySchema::EntitySchema(const json &entity_schema) {
        // TODO validate schema first ...
        m_schema = entity_schema;
    }

    EntitySchema::EntitySchema(const Entity &entity): EntitySchema(entity.schema()) {

    }

    Entity EntitySchema::toEntity() const {
        return Entity{m_schema};
    }

    std::string EntitySchema::name() const { return m_name; }

    EntitySchema & EntitySchema::setName(const std::string &name) {
        m_name = name;
        return *this;
    }

    std::string EntitySchema::type() const { return m_type; }

    EntitySchema & EntitySchema::setType(const std::string &type) {
        if (type=="base" || type=="auth" || type=="view")
            throw std::invalid_argument("Type should either be `base`, `auth` or `view` only.");

        m_type = type;
        return *this;
    }

    bool EntitySchema::hasApi() const { return m_hasApi; }

    EntitySchema & EntitySchema::setHasApi(const bool &hasApi) {
        m_hasApi = hasApi;
        return *this;
    }

    bool EntitySchema::isSystem() const { return m_isSystem; }

    EntitySchema & EntitySchema::setType(const bool &isSystem) {
        m_isSystem = isSystem;
        return *this;
    }

    std::string EntitySchema::listRule() const { return m_listRule; }

    EntitySchema & EntitySchema::setListRule(const std::string &listRule) {
        m_listRule = listRule;
        return *this;
    }

    std::string EntitySchema::getRule() const { return m_getRule; }

    EntitySchema & EntitySchema::setGetRule(const std::string &getRule) {
        m_getRule = getRule;
        return *this;
    }

    std::string EntitySchema::addRule() const { return m_addRule; }

    EntitySchema & EntitySchema::setAddRule(const std::string &addRule) {
        m_addRule = addRule;
        return *this;
    }

    std::string EntitySchema::updateRule() const { return m_updateRule; }

    EntitySchema & EntitySchema::setUpdateRule(const std::string &updateRule) {
        m_updateRule = updateRule;
        return *this;
    }

    std::string EntitySchema::deleteRule() const { return m_deleteRule; }

    EntitySchema & EntitySchema::setDeleteRule(const std::string &deleteRule) {
        m_deleteRule = deleteRule;
        return *this;
    }

    std::vector<EntitySchemaField> EntitySchema::fields() const { return m_fields; }

    EntitySchema & EntitySchema::addField(const EntitySchemaField &field) {
        m_fields.push_back(field);
        return *this;
    }
} // mantis