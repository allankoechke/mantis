//
// Created by codeart on 12/11/2025.
//

#include "../../../include/mantis/core/models/entity_schema.h"

#include "mantis/core/exceptions.h"

namespace mantis {
    EntitySchema::EntitySchema(const std::string &entity_name, const std::string &entity_type) {
        setName(entity_name).setType(entity_type);
    }

    EntitySchema::EntitySchema(const EntitySchema &other) = default;

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
        return EntitySchema::genEntityId(m_name);
    }

    EntitySchemaField &EntitySchema::field(const std::string &field_name) {
        if (field_name.empty()) throw std::invalid_argument("Empty field name.");

        const auto it = std::ranges::find_if(m_fields, [field_name](const auto &field) {
            return field_name == field.name();
        });

        if (it == m_fields.end())
            throw std::out_of_range("Field not found for name `" + field_name + "`");

        return *it;
    }

    EntitySchemaField &EntitySchema::fieldById(const std::string &field_id) {
        if (field_id.empty()) throw std::invalid_argument("Empty field id.");

        const auto it = std::ranges::find_if(m_fields, [field_id](const auto &field) {
            return field_id == field.id();
        });

        if (it == m_fields.end())
            throw std::out_of_range("Field not found for id `" + field_id + "`");

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

    void EntitySchema::updateWith(const nlohmann::json &new_data) {
        if (new_data.empty()) return;

        if (new_data.contains("deleted_fields")) {
            for (const auto &field: new_data["deleted_fields"]) {
                removeField(field.get<std::string>());
            }
        }

        if (new_data.contains("name")) {
            setName(new_data["name"].get<std::string>());
        }

        if (new_data.contains("type")) {
            setType(new_data["type"].get<std::string>());
        }

        if (new_data.contains("system"))
            setSystem(new_data.at("system").get<bool>());

        if (new_data.contains("has_api"))
            setHasApi(new_data.at("has_api").get<bool>());

        if (new_data.contains("list_rule"))
            setListRule(new_data.at("list_rule").get<std::string>());

        if (new_data.contains("get_rule"))
            setListRule(new_data.at("get_rule").get<std::string>());

        if (new_data.contains("add_rule"))
            setListRule(new_data.at("add_rule").get<std::string>());

        if (new_data.contains("update_rule"))
            setListRule(new_data.at("update_rule").get<std::string>());

        if (new_data.contains("delete_rule"))
            setListRule(new_data.at("delete_rule").get<std::string>());

        // 'auth' and 'base' types have 'fields' key ...
        if ((m_type == "base" || m_type == "auth") && new_data.contains("fields") && new_data["fields"].
            is_array()) {
            for (const auto &field: new_data["fields"]) {
                const auto name = field.contains("name") && field["name"].is_string()
                                      ? field["name"].get<std::string>()
                                      : "";
                const auto id = field.contains("id") && field["id"].is_string()
                                    ? field["id"].get<std::string>()
                                    : "";

                if (name.empty() && id.empty())
                    throw MantisException(
                        400, "At least field `id` or `name` should be provided for each field entry.");

                if (id.empty() && hasFieldById(id)) {
                    // Update existing item by id
                    auto &f = fieldById(id);
                    f.updateWith(field);
                } else if (!name.empty() && hasField(name)) {
                    // try updating via name
                    auto &f = EntitySchema::field(name);
                    f.updateWith(field);
                } else {
                    // create new field
                    m_fields.emplace_back(field);
                }
            }
        }

        // For 'view' types, check for 'view_query'
        if (m_type == "view" && new_data.contains("view_query") && new_data["view_query"].is_string()
            && !new_data.at("view_query").get<std::string>().empty()) {
            setViewQuery(new_data.at("view_query").get<std::string>());
        }
    }

    std::string EntitySchema::name() const { return m_name; }

    EntitySchema &EntitySchema::setName(const std::string &name) {
        m_name = name;
        return *this;
    }

    std::string EntitySchema::type() const { return m_type; }

    EntitySchema &EntitySchema::setType(const std::string &type) {
        if (!(type == "base" || type == "auth" || type == "view"))
            throw std::invalid_argument("Type should either be `base`, `auth` or `view` only.");

        if (type == "view")
            m_fields.clear(); // Clear all fields if we are turning it to a view type

        else if (type == "base") {
            // Add base fields if they don't exist yet ...
            addFieldsIfNotExist("base");
        } else {
            // Add auth specific system fields ...
            addFieldsIfNotExist("auth");
        }

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
        const auto _ = field.validate();
        m_fields.push_back(field);
        return *this;
    }

    bool EntitySchema::removeField(const std::string &field_name) {
        // If the field does not exist yet, return false
        if (!hasField(field_name)) return false;

        std::erase_if(
            m_fields,
            [&](const EntitySchemaField &field) { return field.name() == field_name; }
        );

        return true;
    }

    nlohmann::json EntitySchema::toJson() const {
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

    std::string EntitySchema::toDDL() const {
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
            if (field.constraints().contains("default_value") && !field.constraints()["default_value"].is_null())
                ddl << " DEFAULT " << toDefaultSqlValue(field.type(), field.constraints()["default_value"]);
        }
        ddl << ");";

        return ddl.str();
    }

    bool EntitySchema::hasField(const std::string &field_name) const {
        if (m_type == "view") return false;

        return std::ranges::find_if(m_fields, [field_name](const EntitySchemaField &field) {
            return field.name() == field_name;
        }) != m_fields.end();
    }

    bool EntitySchema::hasFieldById(const std::string &field_id) const {
        if (field_id.empty()) return false;
        if (m_type == "view") return false;

        return std::ranges::find_if(m_fields, [field_id](const EntitySchemaField &field) {
            return field.id() == field_id;
        }) != m_fields.end();
    }

    std::string EntitySchema::toDefaultSqlValue(const std::string &type, const nlohmann::json &v) {
        if (type.empty())
            throw std::invalid_argument("Required field type can't be empty!");

        if (v.is_null()) return "NULL";

        if (type == "xml" || type == "string") {
            return "'" + v.dump() + "'";
        }

        if (type == "double" || type == "int8" || type == "uint8"
            || type == "int16" || type == "uint16" || type == "int32"
            || type == "uint32" || type == "int64" || type == "uint64"
            || type == "date" || type == "json" || type == "blob"
            || type == "date" || type == "file" || type == "files") {
            return v.dump();
        }

        if (type == "bool") {
            return v.get<bool>() ? "1" : "0";
        }

        throw std::runtime_error("Unsupported field type `" + type + "`");
    }

    std::string EntitySchema::genEntityId(const std::string &entity_name) {
        return "mbt_" + std::to_string(std::hash<std::string>{}(entity_name));
    }

    std::optional<std::string> EntitySchema::validate(const EntitySchema &table_schema) {
        if (trim(table_schema.name()).empty())
            return "Entity schema name is empty!";

        if (!(table_schema.type() == "base" || table_schema.type() == "view" || table_schema.type() == "auth")) {
            return "Entity schema type is invalid!";
        }

        if (table_schema.type() == "view") {
            if (trim(table_schema.viewQuery()).empty())
                return "Entity schema view query is empty!";
            // TODO check if query is a valid SQL view query
        } else {
            // First validate each field
            for (const auto &field: table_schema.fields()) {
                if (auto err = field.validate(); err.has_value())
                    return err.value();
            }

            // Check that base fields are present
            if (table_schema.type() == "base") {
                for (const auto &field_name: EntitySchemaField::defaultBaseFields()) {
                    if (!table_schema.hasField(field_name))
                        return "Entity schema does not have field: `" + field_name + "` required for `" + table_schema.
                               type() + "` types.";
                }
            }

            if (table_schema.type() == "auth") {
                for (const auto &field_name: EntitySchemaField::defaultAuthFields()) {
                    if (!table_schema.hasField(field_name))
                        return "Entity schema does not have field: `" + field_name + "` required for `" + table_schema.
                               type() + "` types.";
                }
            }
        }

        return std::nullopt;
    }

    std::optional<std::string> EntitySchema::validate() const {
        return validate(*this);
    }

    std::string EntitySchema::getFieldType(const std::string &type, std::shared_ptr<soci::session> sql) {
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
        return sql->get_backend()->create_column_type(EntitySchemaField::toSociType(type), 0, 0);
    }

    void EntitySchema::addFieldsIfNotExist(const std::string &type) {
        if (type == "base") {
            for (const auto &field: EntitySchema::defaultBaseFieldsSchema()) {
                if (!hasField(field.name())) {
                    addField(field);
                }
            }
        } else if (type == "auth") {
            for (const auto &field: EntitySchema::defaultAuthFieldsSchema()) {
                if (!hasField(field.name())) {
                    addField(field);
                }
            }
        } else {
            throw MantisException(500, "Operation not supported for `view` types.");
        }
    }
} // mantis
