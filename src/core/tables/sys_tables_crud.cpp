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
#include "../../../include/mantis/utils/utils.h"

#include <soci/soci.h>

#include "mantis/core/router.h"
#include "private/soci-mktime.h"

#define __file__ "core/tables/sys_tables_crud.cpp"

namespace mantis
{
    json SysTablesUnit::create(const json& entity, const json& opts)
    {
        json result;

        try
        {
            const auto name = entity.value("name", "");
            const auto type = entity.value("type", "");
            const auto has_api = entity.value("has_api", true);
            const auto fields = entity.value("fields", json::array());

            // Update rules in the individual table types
            const auto addRule = entity.value("addRule", "");
            const auto getRule = entity.value("getRule", "");
            const auto listRule = entity.value("listRule", "");
            const auto updateRule = entity.value("updateRule", "");
            const auto deleteRule = entity.value("deleteRule", "");

            // Hash the name for the ID
            std::string id = generateTableId(name);

            Log::trace("Creating table: {} with id: {}", name, id);

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
            auto sql =  MantisApp::instance().db().session();
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
                AuthTable auth;
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
                ViewTable view;
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
                BaseTable base;
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
                // Insert to __tables
                *sql <<
                    "INSERT INTO __tables (id, name, type, has_api, schema, created, updated) VALUES (:id, :name, :type, :has_api, :schema, :created, :updated)"
                    ,
                    soci::use(id), soci::use(name), soci::use(type), soci::use(has_api),
                    soci::use(schema_str), soci::use(*created_tm), soci::use(*created_tm);

                // Create actual SQL table
                *sql << table_ddl;
                tr.commit();

                json obj;
                obj["id"] = id;
                obj["name"] = name;
                obj["type"] = type;
                obj["has_api"] = has_api;
                obj["schema"] = json::parse(schema_str);
                obj["created"] = DatabaseUnit::tmToISODate(*created_tm);
                obj["updated"] = DatabaseUnit::tmToISODate(*created_tm);

                result["data"] = obj;

                // Add created table to the routes
                if (auto res = MantisApp::instance().router().addRoute(name);
                    !res.value("success", false))
                {
                    Log::warn("Restart server to get new route changes! {}",
                        res.value("error", ""));
                }
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
        const auto sql =  MantisApp::instance().db().session();

        soci::row r;
        *sql << "SELECT has_api, name, type, schema FROM __tables WHERE id = :id", soci::use(id), soci::into(r);

        if (!sql->got_data())
        {
            return std::nullopt;
        }

        const auto has_api = r.get<bool>(0);
        const auto name = r.get<std::string>(1);
        const auto type = r.get<std::string>(2);
        const auto schema = r.get<json>(3);

        json tb;
        tb["id"] = id;
        tb["name"] = name;
        tb["has_api"] = has_api;
        tb["type"] = type;
        tb["schema"] = schema;

        return tb;
    }

    json SysTablesUnit::update(const std::string& id, const json& entity, const json& opts)
    {
        const auto sql =  MantisApp::instance().db().session();
        soci::transaction tr(*sql);
        json response;
        response["data"] = json::object();
        response["error"] = "";

        try
        {
            // Get Original Object
            soci::row rw;
            *sql << "SELECT id,name,type,schema,has_api FROM __tables WHERE id = :id", soci::use(id), soci::into(rw);

            if (!sql->got_data())
            {
                response["error"] = "Table with that ID was not found!";
                return response;
            }

            // Old data ...
            auto t_id = rw.get<std::string>(0);
            auto t_name = rw.get<std::string>(1);
            auto t_type = rw.get<std::string>(2);
            auto t_schema = rw.get<json>(3);
            auto t_has_api = rw.get<bool>(4);
            std::vector<json> t_fields = t_schema.value("fields", json::array());

            // Just hold this name for later
            const auto old_name = t_name;
            const auto old_type = t_type;

            // For now, we don't support changing types
            if (entity.contains("type") && t_type != entity["type"].get<std::string>())
            {
                response["error"] = "Changing table types is not supported yet!";
                return response;
            }

            // Delete fields ...
            if (const auto delFields = entity.value("deletedFields", std::vector<std::string>{}); !delFields.empty())
            {
                std::vector<std::string> sys_fields{};

                // Drop all columns in this segment ...
                for (const auto& field_name : delFields)
                {
                    // Skip empty fields ..
                    if (trim(field_name).empty())
                    {
                        response["error"] = "Field name can't be empty!";
                        response["status"] = 400;
                        return response;
                    };

                    // We can't drop system fields here, so, lets check for that ...
                    if (t_type == "base") sys_fields = baseFields;
                    else if (t_type == "auth") sys_fields = authFields;
                    // For views, ignore ...

                    // If the field exists in system types, ignore it
                    if (std::find(sys_fields.begin(), sys_fields.end(), field_name) != sys_fields.end())
                        continue;

                    // If the field is valid, generate drop colum statement and execute!
                    *sql << sql->get_backend()->drop_column(t_name, trim(field_name));

                    // Log::trace("Fields array size before removing {} = {}", field_name, t_fields.size());
                    // Remove the field from the array as well.
                    t_fields.erase(std::remove_if(t_fields.begin(), t_fields.end(), [&](const auto& field)
                    {
                        return field.value("name", "") == trim(field_name);
                    }));
                    // Log::trace("Fields array size after removing {} = {}", field_name, t_fields.size());
                }
            }

            for (const auto& field : entity.value("fields", std::vector<json>{}))
            {
                // Log::trace("Field: {}", field.value("name", ""), field.dump());

                // Ensure field name is provided, if not so, throw an error!
                const auto field_name = field.value("name", "");
                if (field_name.empty())
                {
                    response["error"] = "Field name can't be empty!";
                    return response;
                }

                int found_index = -1, i = 0;
                for (const auto& _field : t_fields)
                {
                    // Log::trace("Field: {}", _field.dump());
                    if (_field.value("name", "") == field_name)
                    {
                        found_index = i;
                        break;
                    }
                    i++;
                }

                // If we didn't find any matching field, then, lets treat it like a new
                // field. We shall create a new Field object and alter the table structure.
                if (found_index == -1)
                {
                    json field_opts;
                    if (field.contains("autoGeneratePattern"))
                        field_opts["autoGeneratePattern"] = field.value("autoGeneratePattern", "");

                    if (field.contains("defaultValue"))
                        field_opts["defaultValue"] = field.value("defaultValue", "");

                    if (field.contains("maxValue"))
                        field_opts["maxValue"] = field.value("maxValue", "");

                    if (field.contains("minValue"))
                        field_opts["minValue"] = field.value("minValue", "");

                    if (field.contains("validator"))
                        field_opts["validator"] = field.value("validator", "");


                    if (field.contains("unique"))
                        field_opts["unique"] = field.value("unique", "");

                    // Extract field data
                    auto field_primaryKey = field.value("primaryKey", false);
                    auto field_required = field.value("required", false);
                    auto field_system = false;
                    auto field_typeStr = field.value("type", "");
                    auto field_unique = field.value("unique", false);
                    const auto field_type = getFieldType(field_typeStr);

                    // Ensure field type is provided
                    if (!field_type.has_value())
                    {
                        response["error"] = "Field type for " + field_name + " is required!";
                        return response;
                    }

                    // Create field item ...
                    Field f{field_name, field_type.value(), field_required, field_primaryKey, field_system, field_opts};
                    t_fields.push_back(f.to_json());

                    // Execute SQL to create the column (field)
                    std::string query = sql->get_backend()->add_column(t_name, field_name, f.toSociType(), 0, 0);
                    query += field_required ? " NOT NULL" : ""; // Add Not Null constraint
                    *sql << query;

                    // Add unique constraint to the created column
                    if (field_unique)
                        *sql << "ALTER TABLE " + t_name + " ADD " + sql->get_backend()->constraint_unique(
                            "unique_" + field_name, field_name);
                }
                else
                {
                    // Get the found json object, we'll use it to update data before dumping it back.
                    auto old_field = t_fields.at(found_index);

                    if (field.contains("autoGeneratePattern"))
                        old_field["autoGeneratePattern"] = field["autoGeneratePattern"];

                    if (field.contains("defaultValue"))
                        old_field["defaultValue"] = field["defaultValue"];

                    if (field.contains("maxValue"))
                        old_field["maxValue"] = field["maxValue"];

                    if (field.contains("minValue"))
                        old_field["minValue"] = field["minValue"];

                    if (field.contains("validator"))
                        old_field["validator"] = field["validator"];

                    if (field.contains("unique"))
                    {
                        const auto is_unique = field.value("unique", false);
                        old_field["unique"] = is_unique;

                        if (is_unique)
                        {
                            *sql << ("ALTER TABLE " + t_name + " ADD " + sql->get_backend()->constraint_unique(
                                "unique_" + field_name, field_name));
                        }

                        // Removing unique constraint is database specific
                        // TODO look into removing unique constraint later
                    }

                    if (field.contains("new_name"))
                    {
                        const auto new_name = field.value("new_name", "");
                        old_field["name"] = new_name;

                        if (new_name.empty())
                        {
                            tr.rollback();
                            response["error"] = "Field new_name can't be empty!";
                            return response;
                        }

                        // TODO maybe check that the table name doesn't exist yet, but, the db will throw an error

                        std::string backend_name = sql->get_backend()->get_backend_name();
                        std::string rename_sql = "ALTER TABLE ";
                        rename_sql.append(t_name).append(" RENAME COLUMN ").append(field_name).append(" TO ").append(new_name);

                        // Execute rename SQL query
                        *sql << rename_sql;
                    }

                    if (field.contains("type"))
                    {
                        const auto t = old_field.value("type", "");
                        // Only if the field type is changed, avoid throwing errors for unchanged types
                        if (t != field.value("type", ""))
                        {
                            // SQLite does not support this type changes, for now, maybe look at it later
                            // TODO ..
                            if (sql->get_backend_name() == "sqlite3")
                            {
                                response["error"] = "Changing column type not supported in SQLite databases yet!";
                                response["status"] = 500;
                                return response;
                            }

                            if (t.empty())
                            {
                                tr.rollback();
                                response["error"] = "Field type for " + field_name + " can't be empty!";
                                return response;
                            }

                            auto f_type = getFieldType(old_field.at("type").get<std::string>());
                            if (!f_type.has_value())
                            {
                                tr.rollback();
                                response["error"] = "Unsupported field type for column " + field_name + "!";
                                return response;
                            }

                            // TODO test on all database types
                            // Fails on SQLite?
                            Log::trace("Attempt to change column type ...");

                            // Update field data type in our json object ...
                            old_field["type"] = t;
                            sql->alter_column(
                                t_name,
                                old_field.at("name").get<std::string>(),
                                Field::toSociType(f_type.value()));
                        }
                    }

                    auto _type = getFieldType(old_field.value("type", ""));
                    if (!_type.has_value())
                    {
                        tr.rollback();
                        Log::critical("Error parsing field type of {} in {}", old_field.value("type", ""), field_name);

                        response["error"] = "Error parsing field type!";
                        return response;
                    }

                    // Create new field object with the updated data then dump it back to the `t_fields` array.
                    Field f{
                        old_field.at("name").get<std::string>(),
                        _type.value(),
                        old_field.value("required", false),
                        old_field.value("primaryKey", false),
                        false,
                        old_field
                    };

                    // Replace field in place in the array
                    t_fields[found_index] = f.to_json();
                }
            }

            // Update has_api field ...
            if (entity.contains("has_api"))
            {
                t_has_api = entity.at("has_api").get<bool>();
                t_schema["has_api"] = t_has_api;
            }

            // Update access rules if passed in
            if (entity.contains("addRule")) t_schema["addRule"] = entity.value("addRule", "");
            if (entity.contains("getRule")) t_schema["getRule"] = entity.value("getRule", "");
            if (entity.contains("listRule")) t_schema["listRule"] = entity.value("listRule", "");
            if (entity.contains("updateRule")) t_schema["updateRule"] = entity.value("updateRule", "");
            if (entity.contains("deleteRule")) t_schema["deleteRule"] = entity.value("deleteRule", "");

            // If we are changing table names, then ensure it's not empty nor system name kind of ...
            if (const auto& name = trim(entity.value("name", ""));
                !name.empty() && name != t_name)
            {
                // Hold system table names ...
                const std::vector<std::string> sys_tables{"__admin", "__tables"};

                // Check that the new name is not matching any system table names
                if (std::find(sys_tables.begin(), sys_tables.end(), name) != sys_tables.end())
                {
                    response["error"] = "The selected table name '" + name + "' is system reserved!";
                    return response;
                }

                // Update table name
                // TODO check if this syntax works for all db types
                // Works on SQLite
                *sql << "ALTER TABLE " + t_name + " RENAME TO " + name;

                // Create new table ID and update the json object
                const auto nId = generateTableId(name);
                t_schema["id"] = nId;
                t_schema["name"] = name;
                t_id = nId;
                t_name = name;
            }

            // Update fields ...
            t_schema["fields"] = t_fields; // Create default time values

            // Get updated timestamp
            std::time_t t = time(nullptr);
            std::tm* updated_tm = std::localtime(&t);

            // Update table record, if all went well.
            std::string query = "UPDATE __tables SET id = :id, name = :name, type = :type, schema = :schema,";
            query += " has_api = :has_api, updated = :updated WHERE id = :old_id";

            *sql << query, soci::use(t_id), soci::use(t_name), soci::use(t_type), soci::use(t_schema),
                soci::use(t_has_api), soci::use(*updated_tm), soci::use(id);

            // Write out any pending changes ...
            tr.commit();

            // Fetch the new record and return it to the user ...
            soci::row r;
            *sql << ("SELECT id,name,type,schema,has_api,created,updated FROM __tables WHERE id = :id"),
                soci::use(t_id), soci::into(r);

            // Get the data into a json object, if it was found ...
            if (sql->got_data())
            {
                json record;
                record["id"] = r.get<std::string>(0);
                record["name"] = r.get<std::string>(1);
                record["type"] = r.get<std::string>(2);
                record["schema"] = r.get<json>(3);
                record["has_api"] = r.get<bool>(4);
                record["created"] = r.get<std::string>(5);
                record["updated"] = r.get<std::string>(6);

                response["data"] = record;

                // Let's update our local cache with the new information
                fromJson(r.get<json>(3)); // Pass in our schema to update all fields
            }

            // Only trigger routes to be reloaded if table name changes.
            if (t_name != old_name)
            {
                // Update route for this table
                const json obj {
                    {"new_name", t_name},
                    {"old_name", old_name},
                    {"old_type", old_type}
                };

                Log::trace("Update With: {}", obj.dump());
                if (auto res = MantisApp::instance().router().updateRoute(obj);
                    !res.value("success", false))
                {
                    Log::warn("Restart server to get new route changes! {}",
                        res.value("error", ""));
                }
            }
        }
        catch (std::exception& e)
        {
            response["error"] = e.what();
            response["status"] = 500;

            Log::critical("Error Updating Table Schema: {}", e.what());
        }

        return response;
    }

    bool SysTablesUnit::remove(const std::string& id, const json& opts)
    {
        const auto sql =  MantisApp::instance().db().session();
        soci::transaction tr(*sql);

        json response;

        // Check if item exists of given id
        std::string name, type;
        *sql << "SELECT name, type FROM __tables WHERE id = :id",
        soci::use(id), soci::into(name), soci::into(type);

        if (!sql->got_data())
        {
            throw std::runtime_error("Item with id = '" + id + "' was not found!");
        }

        // Remove from DB
        *sql << "DELETE FROM __tables WHERE id = :id", soci::use(id);
        *sql << "DROP TABLE IF EXISTS " + name;

        tr.commit();

        // Update route for this table
        const json obj {{"name", name}, {"type", type}};
        if (const auto res = MantisApp::instance().router().removeRoute(obj);
            !res.value("success", false))
        {
            Log::warn("Restart server to get new route changes! {}",
                res.value("error", ""));
        }

        return true;
    }

    std::vector<json> SysTablesUnit::list(const json& opts)
    {
        const auto sql =  MantisApp::instance().db().session();
        const soci::rowset<soci::row> rs = (sql->prepare << "SELECT id, name, type, schema, has_api, created, updated FROM __tables");
        nlohmann::json response = nlohmann::json::array();

        for (const auto& row : rs)
        {
            const auto id = row.get<std::string>(0);
            const auto name = row.get<std::string>(1);
            const auto type = row.get<std::string>(2);
            const auto schema_json = row.get<json>(3);
            const auto has_api = row.get<bool>(4);
            const auto created = row.get<std::string>(5);
            const auto updated = row.get<std::string>(6);

            json tb;
            tb["id"] = id;
            tb["name"] = name;
            tb["type"] = type;
            tb["has_api"] = has_api;
            tb["schema"] = schema_json;
            tb["created"] = created;
            tb["updated"] = updated;

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
            const auto sql =  MantisApp::instance().db().session();
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
