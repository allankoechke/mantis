//
// Created by allan on 18/05/2025.
//

#include "../../../include/mantis/core/tables/sys_tables.h"
#include "../../../include/mantis/app/app.h"
#include "../../../include/mantis/core/crud/crud.h"
#include "../../../include/mantis/core/database.h"
#include "../../../include/mantis/core/models/models.h"
#include "../../../include/mantis/core/tables/tables.h"
#include "../../../include/mantis/core/logging.h"
#include "../../../include/mantis/utils.h"

#include <soci/soci.h>
#include "private/soci-mktime.h"


namespace mantis
{
    json SysTablesUnit::create(const json& entity, const json& opts)
    {
        json result;

        try
        {
            const auto name = entity.value("name", "");
            const auto type = entity.value("type", "");
            const auto fields = entity.value("fields", json::array());

            // Update rules in the individual table types
            const auto addRule = entity.value("addRule", "");
            const auto getRule = entity.value("getRule", "");
            const auto listRule = entity.value("listRule", "");
            const auto updateRule = entity.value("updateRule", "");
            const auto deleteRule = entity.value("deleteRule", "");

            // Hash the name for the ID
            std::string id = TableUnit::generateTableId(name);

            Log::debug("Creating table: {} with id: {}", name, id);

            // Check if item exits already in db
            if (itemExists(name, id))
            {
                result["error"] = "Table with similar name exists.";
                result["status"] = 400;
                return result;
            }

            Log::trace("Table: {} will be created", name);

            // Create default time values
            std::time_t t = time(nullptr);
            std::tm* created_tm = std::localtime(&t);

            // Database session & transaction instance
            auto sql = m_app->db().session();
            soci::transaction tr(*sql);

            std::string schema_str, table_ddl;
            std::vector<Field> new_fields, rules_fields;

            for (const auto& field : fields)
            {
                Log::trace("Field: {} data = {}", field.value("name", ""), field.dump());
                auto _autoGeneratePattern = field.value("autoGeneratePattern", "");
                auto _defaultValue = field.value("defaultValue", "");
                auto _maxValue = field.value("maxValue", "");
                auto _minValue = field.value("minValue", "");
                const auto _name = field.value("name", "");
                auto _primaryKey = field.value("primaryKey", false);
                auto _required = field.value("required", false);
                auto _system = false;
                auto _typeStr = field.value("type", "");
                const auto _type = getFieldType(_typeStr);

                // Ensure field name is provided
                if (trim(_name).empty())
                {
                    result["error"] = "Field type 'name' can't be empty";
                    result["status"] = 400;
                    return result;
                }

                // Ensure field type is provided
                if (!_type.has_value())
                {
                    result["error"] = "Field type '" + _typeStr + "' not recognised";
                    result["status"] = 400;
                    return result;
                }


                Field f{_name, _type.value(), _required, _primaryKey, _system};
                new_fields.push_back(f);
            }

            if (type == "auth")
            {
                AuthTable auth(m_app.get());
                auth.id = id;
                auth.name = name;
                auth.system = false;
                auth.addRule = addRule;
                auth.getRule = getRule;
                auth.updateRule = updateRule;
                auth.deleteRule = deleteRule;
                auth.listRule = listRule;

                // Default values
                // "id", "created", "updated", "email", "password", "name"
                for (const auto& field : new_fields)
                {
                    if (fieldExists(auth.type, field.name)) continue;
                    auth.fields.push_back(field);
                }

                schema_str = auth.to_json().dump();
                table_ddl = auth.to_sql();
            }
            else if (type == "view")
            {
                ViewTable view(m_app.get());
                view.id = id;
                view.name = name;
                view.system = false;
                view.getRule = getRule;
                view.listRule = listRule;

                // For view types, we need the SQL query
                auto _sourceSQL = entity.value("sql", "");

                // Ensure SQL query is provided
                if (_sourceSQL.empty())
                {
                    result["error"] = "View SQL Query is empty";
                    result["status"] = 400;
                    return result;
                }

                view.sourceSQL = _sourceSQL;
                schema_str = view.to_json().dump();
                table_ddl = view.to_sql();
            }
            else
            {
                BaseTable base(m_app.get());
                base.id = id;
                base.name = name;
                base.system = false;
                base.addRule = addRule;
                base.getRule = getRule;
                base.updateRule = updateRule;
                base.deleteRule = deleteRule;
                base.listRule = listRule;

                // Default fields
                // "id", "created", "updated"

                for (const auto& field : new_fields)
                {
                    if (fieldExists(base.type, field.name)) continue;
                    base.fields.push_back(field);
                }

                schema_str = base.to_json().dump();
                table_ddl = base.to_sql();
            }

            // Execute DDL & Save to DB
            Log::debug("Schema: {}", schema_str);
            Log::debug("Table DDL: {}", table_ddl);

            try
            {
                int has_api = 1;
                // Insert to __tables
                *sql <<
                    "INSERT INTO __tables (id, name, type, has_api, schema, created, updated) VALUES (:id, :name, :type, :has_api, :schema, :created, :updated)"
                    ,
                    soci::use(id), soci::use(name), soci::use(type), soci::use(has_api),
                    soci::use(schema_str), soci::use(*created_tm), soci::use(*created_tm);

                // Create actual SQL table
                *sql << table_ddl;

                json obj;
                obj["id"] = id;
                obj["name"] = name;
                obj["type"] = type;
                obj["fields"] = json::array();
                obj["created"] = DatabaseUnit::tmToISODate(*created_tm);
                obj["updated"] = DatabaseUnit::tmToISODate(*created_tm);
                obj["addRule"] = addRule;
                obj["deleteRule"] = deleteRule;
                obj["getRule"] = getRule;
                obj["listRule"] = listRule;
                obj["updateRule"] = updateRule;

                // Dump complete field information, not just the passed in values
                for (const auto& field : new_fields) { obj["fields"].push_back(field.to_json()); }

                result["data"] = obj;

                tr.commit();
            }
            catch (const soci::soci_error& e)
            {
                tr.rollback();

                result["error"] = e.what();
                result["status"] = 500;
                return result;
            } catch (const std::exception& e)
            {
                tr.rollback();

                result["error"] = e.what();
                result["status"] = 500;
                return result;
            }

            return result;
        }
        catch (std::exception& e)
        {
            Log::critical("SysTablesUnit::SysTablesUnit: {}", e.what());
            json err;
            err["error"] = e.what();
            err["status"] = 500;
            result["error"] = err;
            return result;
        }
    }

    std::optional<json> SysTablesUnit::read(const std::string& id, const json& opts)
    {
        const auto sql = m_app->db().session();

        soci::row r;
        *sql << "SELECT id, name, type, schema FROM __tables WHERE id = :id", soci::use(id), soci::into(r);

        if (!sql->got_data())
        {
            return std::nullopt;
        }

        // const auto id = r.get<std::string>(0);
        const auto name = r.get<std::string>(1);
        const auto type = r.get<std::string>(2);
        const auto schema = r.get<json>(3);

        json tb;
        tb["id"] = id;
        tb["name"] = name;
        tb["type"] = type;
        tb["schema"] = schema;

        return tb;
    }

    json SysTablesUnit::update(const std::string& id, const json& entity, const json& opts)
    {
        const auto sql = m_app->db().session();
        soci::transaction tr(*sql);
        json response;
        response["data"] = json::object();
        response["error"] = json::object();

        // Get Original Object
        soci::row rw;
        *sql << "SELECT * FROM __tables WHERE id = :id", soci::use(id), soci::into(rw);

        if (!sql->got_data())
        {
            json err;
            err["message"] = "Table with that ID does not exist!";
            response["error"] = err;
            return response;
        }

        const auto old_id = rw.get<std::string>(0);
        const auto old_name = rw.get<std::string>(1);
        const auto old_type = rw.get<std::string>(2);
        const auto old_schema_json = rw.get<std::string>(3);
        const json old_schema_obj = json::parse(old_schema_json);

        if (entity.contains("name") && entity.value("name", "") != old_name)
        {
            const auto name = entity.value("name", "");
            const auto nId = TableUnit::generateTableId(name);

            // Update Name & ID in the __tables catalogue
            *sql << "UPDATE __tables SET id = :id & name = :name WHERE id = :old_id",
                soci::use(nId), soci::use(name), soci::use(id);

            // Update the
            // TODO LATER
        }

        std::string new_schema;

        // Update schema
        *sql << "UPDATE __tables SET schema = :schema WHERE id = :id",
            soci::use(new_schema), soci::use(id);

        // TODO: Compare fields and generate ALTER TABLE statements
        // Simple naive strategy: just add missing columns
        // for (const auto& newField : updated->fields) {
        //     bool exists = false;
        //     for (const auto& oldField : existing->fields) {
        //         if (newField.name == oldField.name) {
        //             exists = true;
        //             break;
        //         }
        //     }
        //     if (!exists) {
        //         *sql << "ALTER TABLE " << updated->name << " ADD COLUMN " << newField.to_sql();
        //     }
        // }

        tr.commit();
        return response;
    }

    bool SysTablesUnit::remove(const std::string& id, const json& opts)
    {
        const auto sql = m_app->db().session();
        soci::transaction tr(*sql);

        json response;

        // Check if item exists of given id
        std::string name;
        *sql << "SELECT name FROM __tables WHERE id = :id", soci::use(id), soci::into(name);

        if (!sql->got_data())
        {
            throw std::runtime_error("Item with id = '" + id + "' was not found!");
        }

        // Remove from DB
        *sql << "DELETE FROM __tables WHERE id = :id", soci::use(id);
        *sql << "DROP TABLE IF EXISTS " + name;

        tr.commit();

        // TODO reload routes

        return true;
    }

    std::vector<json> SysTablesUnit::list(const json& opts)
    {
        const auto sql = m_app->db().session();
        const soci::rowset<soci::row> rs = (sql->prepare << "SELECT id, name, type, schema, has_api FROM __tables");
        nlohmann::json response = nlohmann::json::array();

        for (const auto& row : rs)
        {
            const auto id = row.get<std::string>(0);
            const auto name = row.get<std::string>(1);
            const auto type = row.get<std::string>(2);
            const auto schema_json = row.get<std::string>(3);
            const auto has_api = row.get<bool>(4);

            // const json j = json::parse(schema_json);

            json tb;
            tb["id"] = id;
            tb["name"] = name;
            tb["type"] = type;
            tb["has_api"] = has_api;
            tb["schema"] = schema_json;

            response.push_back(tb);
        }

        return response;
    }

    bool SysTablesUnit::itemExists(const std::string& tableName, const std::string& id) const
    {
        Log::trace("SysTablesUnit::itemExists for {} {}", tableName, id);
        try
        {
            int count;
            const auto sql = m_app->db().session();
            const std::string query = "SELECT COUNT(id) FROM " + tableName + " WHERE id = :id";
            *sql << query, soci::use(id), soci::into(count);
            return sql->got_data();
        }
        catch (soci::soci_error& e)
        {
            Log::trace("SysTablesUnit::itemExists error: {}", e.what());
            return false;
        }
    }
} // mantis
