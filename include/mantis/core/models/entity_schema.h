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
        EntitySchema(const EntitySchema&);
        EntitySchema& operator=(const EntitySchema&);
        EntitySchema(EntitySchema&&);
        EntitySchema& operator=(EntitySchema&&);
        ~EntitySchema();

        explicit EntitySchema(const json& entity_schema);

        explicit EntitySchema(const Entity& entity);

        Entity toEntity() const;

        std::string name() const;

        EntitySchema& setName(const std::string& name);

        std::string type() const;

        EntitySchema& setType(const std::string& type);

        bool hasApi() const;

        EntitySchema& setHasApi(const bool& hasApi);

        bool isSystem() const;

        EntitySchema& setType(const bool& isSystem);

        std::string listRule() const;

        EntitySchema& setListRule(const std::string& listRule);

        std::string getRule() const;

        EntitySchema& setGetRule(const std::string& getRule);

        std::string addRule() const;

        EntitySchema& setAddRule(const std::string& addRule);

        std::string updateRule() const;

        EntitySchema& setUpdateRule(const std::string& updateRule);

        std::string deleteRule() const;

        EntitySchema& setDeleteRule(const std::string& deleteRule);

        std::vector<EntitySchemaField> fields() const;

        EntitySchema& addField(const EntitySchemaField& field);

    private:
        json m_schema;

        std::string m_name;
        std::string m_type;
        bool m_isSystem = false;
        bool m_hasApi = true;

        std::vector<EntitySchemaField> m_fields;

        std::string m_listRule="", m_getRule, m_addRule="", m_updateRule="", m_deleteRule="";
    };
} // mantis

#endif //MANTISBASE_ENTITY_SCHEMA_H