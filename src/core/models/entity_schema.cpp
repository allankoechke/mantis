//
// Created by codeart on 12/11/2025.
//

#include "../../../include/mantis/core/models/entity_schema.h"

namespace mantis {
    EntitySchema::EntitySchema() = default;

    EntitySchema::EntitySchema(const EntitySchema &other)
        : m_name(other.m_name),
          m_type(other.m_type),
          m_viewSqlQuery(other.m_viewSqlQuery),
          m_isSystem(other.m_isSystem),
          m_hasApi(other.m_hasApi),
          m_fields(other.m_fields),
          m_listRule(other.m_listRule),
          m_getRule(other.m_getRule),
          m_addRule(other.m_addRule),
          m_updateRule(other.m_updateRule),
          m_deleteRule(other.m_deleteRule) {
    }

    EntitySchema &EntitySchema::operator=(const EntitySchema &other) {
        if (this != &other) {
            m_name = other.m_name;
            m_type = other.m_type;
            m_viewSqlQuery = other.m_viewSqlQuery;
            m_isSystem = other.m_isSystem;
            m_hasApi = other.m_hasApi;
            m_fields = other.m_fields;
            m_listRule = other.m_listRule;
            m_getRule = other.m_getRule;
            m_addRule = other.m_addRule;
            m_updateRule = other.m_updateRule;
            m_deleteRule = other.m_deleteRule;
        }
        return *this;
    }

    EntitySchema::~EntitySchema() = default;

    EntitySchema::EntitySchema(const json &entity_schema) {
        if (!entity_schema.contains("name") || !entity_schema.contains("type"))
            throw std::invalid_argument("Missing required fields `name` and `type` in schema!");

        const auto type = entity_schema.at("type").get<std::string>();
        setName(entity_schema.at("name").get<std::string>()).setType(type);

        if (entity_schema.contains("system") && entity_schema["system"].is_boolean())
            setSystem(entity_schema.at("system").get<bool>());

        if (entity_schema.contains("has_api") && entity_schema["has_api"].is_boolean())
            setHasApi(entity_schema.at("has_api").get<bool>());

        if (entity_schema.contains("list_rule") && entity_schema["list_rule"].is_string())
            setListRule(entity_schema.at("list_rule").get<std::string>());

        if (entity_schema.contains("get_rule") && entity_schema["get_rule"].is_string())
            setListRule(entity_schema.at("get_rule").get<std::string>());

        if (entity_schema.contains("add_rule") && entity_schema["add_rule"].is_string())
            setListRule(entity_schema.at("add_rule").get<std::string>());

        if (entity_schema.contains("update_rule") && entity_schema["update_rule"].is_string())
            setListRule(entity_schema.at("update_rule").get<std::string>());

        if (entity_schema.contains("delete_rule") && entity_schema["delete_rule"].is_string())
            setListRule(entity_schema.at("delete_rule").get<std::string>());

        // 'auth' and 'base' types have 'fields' key ...
        if ((type == "base" || type == "auth") && entity_schema.contains("fields") && entity_schema["fields"].
            is_array()) {
            m_fields.emplace_back(EntitySchemaField(entity_schema["fields"]));
        }

        // For 'view' types, check for 'view_query'
        if (type == "view" && entity_schema.contains("view_query") && entity_schema["view_query"].is_string()
            && !entity_schema.at("view_query").get<std::string>().empty()) {
            setViewQuery(entity_schema.at("view_query").get<std::string>());
        }
    }

    EntitySchema::EntitySchema(const Entity &entity) : EntitySchema(entity.schema()) {
    }

    Entity EntitySchema::toEntity() const {
        return Entity{toJson()};
    }

    std::string EntitySchema::id() const {
        if (m_name.empty()) throw std::invalid_argument("Expected table name is empty!");
        return "mt_" + std::to_string(std::hash<std::string>{}(m_name));
    }

    EntitySchemaField &EntitySchema::field(const std::string &field_name) {
        if (field_name.empty()) throw std::invalid_argument("Empty field name");

        const auto it = std::ranges::find_if(m_fields, [field_name](const auto &field) {
            return field_name == field.name();
        });

        if (it == m_fields.end())
            throw std::out_of_range("Field not found: " + field_name);

        return *it;
    }

    std::string EntitySchema::viewQuery() const {
        return m_viewSqlQuery;
    }

    EntitySchema &EntitySchema::setViewQuery(const std::string &viewQuery) {
        if (viewQuery.empty()) throw std::invalid_argument("Empty view query statement.");
        // TODO check if its a valid SQL query?
        m_viewSqlQuery = viewQuery;
        return *this;
    }

    std::string EntitySchema::name() const { return m_name; }

    EntitySchema &EntitySchema::setName(const std::string &name) {
        m_name = name;
        return *this;
    }

    std::string EntitySchema::type() const { return m_type; }

    EntitySchema &EntitySchema::setType(const std::string &type) {
        if (type == "base" || type == "auth" || type == "view")
            throw std::invalid_argument("Type should either be `base`, `auth` or `view` only.");

        m_type = type;
        return *this;
    }

    bool EntitySchema::hasApi() const { return m_hasApi; }

    EntitySchema &EntitySchema::setHasApi(const bool &hasApi) {
        m_hasApi = hasApi;
        return *this;
    }

    bool EntitySchema::isSystem() const { return m_isSystem; }

    EntitySchema &EntitySchema::setSystem(const bool &isSystem) {
        m_isSystem = isSystem;
        return *this;
    }

    std::string EntitySchema::listRule() const { return m_listRule; }

    EntitySchema &EntitySchema::setListRule(const std::string &listRule) {
        m_listRule = listRule;
        return *this;
    }

    std::string EntitySchema::getRule() const { return m_getRule; }

    EntitySchema &EntitySchema::setGetRule(const std::string &getRule) {
        m_getRule = getRule;
        return *this;
    }

    std::string EntitySchema::addRule() const { return m_addRule; }

    EntitySchema &EntitySchema::setAddRule(const std::string &addRule) {
        m_addRule = addRule;
        return *this;
    }

    std::string EntitySchema::updateRule() const { return m_updateRule; }

    EntitySchema &EntitySchema::setUpdateRule(const std::string &updateRule) {
        m_updateRule = updateRule;
        return *this;
    }

    std::string EntitySchema::deleteRule() const { return m_deleteRule; }

    EntitySchema &EntitySchema::setDeleteRule(const std::string &deleteRule) {
        m_deleteRule = deleteRule;
        return *this;
    }

    std::vector<EntitySchemaField> EntitySchema::fields() const { return m_fields; }

    EntitySchema &EntitySchema::addField(const EntitySchemaField &field) {
        m_fields.push_back(field);
        return *this;
    }

    json EntitySchema::toJson() const {
        json j;
        j["id"] = id();
        j["name"] = m_name;
        j["type"] = m_type;
        j["has_api"] = m_hasApi;
        j["list_rule"] = m_listRule;
        j["get_rule"] = m_getRule;
        j["add_rule"] = m_addRule;
        j["update_rule"] = m_updateRule;
        j["delete_rule"] = m_deleteRule;

        if (m_type == "view") {
            j["view_query"] = m_viewSqlQuery;
        } else {
            j["fields"] = json::array();
            for (const auto &field: m_fields) {
                j["fields"].emplace_back(field.toJson());
            }
        }
        return j;
    }

    json EntitySchema::toDDL() const {
        // Get DB Session
        const auto sql = MantisBase::instance().db().session();

        std::ostringstream ddl;
        ddl << "CREATE TABLE IF NOT EXISTS " << m_name << " (";
        for (size_t i = 0; i < m_fields.size(); ++i) {
            if (i > 0) ddl << ", ";
            const auto field = m_fields[i];

            ddl << field.name()
                    << " "
                    << getFieldType(field.type(), sql);

            if (field.isPrimaryKey()) ddl << " PRIMARY KEY";
            if (field.required()) ddl << " NOT NULL";
            if (field.isUnique()) ddl << " UNIQUE";
            // if (field.constraints().contains("default_value") && field.constraints()["default_value"].is_null())
            //     ddl << " DEFAULT '" << field.constraints()["default_value"] << "'";
            // TODO add default values conversion
        }
        ddl << ");";

        return ddl.str();
    }

    bool EntitySchema::fieldExists(const std::string &type, const std::string &fieldName) const {
        // Find if field exists ...
        const auto contains = [&](const std::vector<std::string> &vec, const std::string &field_name) -> bool {
            return std::ranges::find(vec, field_name) != vec.end();
        };

        if (m_type == "view") return false;
        if (m_type == "base")
            return contains(EntitySchemaField::defaultBaseFields, fieldName);
        if (m_type == "auth")
            return contains(EntitySchemaField::defaultBaseFields, fieldName);
        return false;
    }

    std::string EntitySchema::getFieldType(const std::string &type, const std::shared_ptr<soci::session> &sql) {
        const auto db_type = sql->get_backend()->get_backend_name();
        // For date types, enforce `text` type SQLite ONLY
        if (db_type == "sqlite3" && type == "date") {
            return "text";
        }

        // PostgreSQL has no support for `db_uint` and `db_uint8` so lets store them in `db_uint16`/`db_int16 types`
        if (db_type == "postgresql" && (type == "uint8" || type == "int8")) {
            return type == "uint8"
                       ? sql->get_backend()->create_column_type(soci::db_uint16, 0, 0)
                       : sql->get_backend()->create_column_type(soci::db_int16, 0, 0);
        }

        // Convert bool types to `db_uint16`
        if (db_type == "postgresql" && type == "bool") {
            return sql->get_backend()->create_column_type(soci::db_uint16, 0, 0);
        }

        // General catch for all other types
        return sql->get_backend()->create_column_type(EntitySchema::toSociType(type), 0, 0);
    }
} // mantis
