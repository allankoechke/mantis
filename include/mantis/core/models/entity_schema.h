//
// Created by codeart on 12/11/2025.
//

#ifndef MANTISBASE_ENTITY_SCHEMA_H
#define MANTISBASE_ENTITY_SCHEMA_H

#include <vector>
#include "entity.h"
#include "entity_schema_field.h"

namespace mantis {
    class EntitySchema {
    public:
        EntitySchema();

        // Allow copy constructors & assignment ops for easy cloning
        EntitySchema(const EntitySchema &);
        EntitySchema &operator=(const EntitySchema &);

        // Delete move constructors
        EntitySchema(EntitySchema &&) = delete;
        EntitySchema &operator=(EntitySchema &&) = delete;

        ~EntitySchema();

        explicit EntitySchema(const json &entity_schema);

        explicit EntitySchema(const Entity &entity);

        Entity toEntity() const;

        // ----------- SCHEMA METHODS ----------- //
        std::string id() const;

        std::string name() const;

        EntitySchema &setName(const std::string &name);

        std::string type() const;

        EntitySchema &setType(const std::string &type);

        bool hasApi() const;

        EntitySchema &setHasApi(const bool &hasApi);

        bool isSystem() const;

        EntitySchema &setSystem(const bool &isSystem);

        std::string listRule() const;

        EntitySchema &setListRule(const std::string &listRule);

        std::string getRule() const;

        EntitySchema &setGetRule(const std::string &getRule);

        std::string addRule() const;

        EntitySchema &setAddRule(const std::string &addRule);

        std::string updateRule() const;

        EntitySchema &setUpdateRule(const std::string &updateRule);

        std::string deleteRule() const;

        EntitySchema &setDeleteRule(const std::string &deleteRule);

        std::vector<EntitySchemaField> fields() const;

        EntitySchema &addField(const EntitySchemaField &field);

        EntitySchemaField &field(const std::string &field_name);

        std::string viewQuery() const;

        EntitySchema &setViewQuery(const std::string &viewQuery);

        // ----------- SCHEMA CONVERSION ----------- //
        json toJson() const;

        json toDDL() const;

        bool fieldExists(const std::string& type, const std::string& fieldName) const;

    private:
        std::string getFieldType(const std::string &type, const std::shared_ptr<soci::session> &sql);

        std::string m_name;
        std::string m_type;
        std::string m_viewSqlQuery;
        bool m_isSystem = false;
        bool m_hasApi = true;

        std::vector<EntitySchemaField> m_fields;

        std::string m_listRule = "", m_getRule = "", m_addRule = "", m_updateRule = "", m_deleteRule = "";
    };
} // mantis

#endif //MANTISBASE_ENTITY_SCHEMA_H
