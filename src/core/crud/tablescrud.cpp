//
// Created by allan on 18/05/2025.
//

#include "../../../include/mantis/core/crud/tablescrud.h"
#include "../../../include/mantis/app/app.h"
#include "../../../include/mantis/core/database.h"
#include "../../../include/mantis/core/models/models.h"
#include "../../../include/mantis/core/logging.h"
#include <soci/soci.h>


namespace mantis
{
    TablesCrud::TablesCrud(MantisApp* app): m_app(app)
    {
    }

    json TablesCrud::create(const json& entity, const json& opts)
    {
        // {
        //     "name": ...,
        //     "type": ..., // view, auth, base
        //     "fields": [
        //         {
        //             "autoGeneratePattern":null,
        //             "defaultValue":null,
        //             "maxValue":null,
        //             "minValue":null,
        //             "name":"id",
        //             "primaryKey":true,
        //             "required":true,
        //             "system":true,
        //             "type":"text"
        //         }
        //     ]
        // }
        std::string name = entity["name"];
        std::string type = entity["type"];
        std::vector<json> fields = entity["fields"];
        std::string id = "mt_" + std::to_string(std::hash<std::string>{}(name)); // Hash the name for the ID

        auto _sourceSQL = entity.value("sql", "");
        auto _systemTable = entity.value("system", false);

        Log::debug("Table: {} with id: {}", name, id);

        std::time_t t = time(nullptr);
        std::tm* created_tm = std::localtime(&t);

        auto sql = m_app->db().session();
        soci::transaction tr(*sql);

        std::string schema_str;
        std::string table_ddl;

        std::vector<Field> new_fields;
        json result;


        for (const auto& field : fields)
        {
            Log::debug("Field: {}", field.value("name", ""));
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

            if (_name.empty())
            {
                json err;
                err["error"] = "Field type 'name' can't be empty";
                result["error"] = err;
                return result;
            }

            if (!_type.has_value())
            {
                json err;
                err["error"] = "Field type '" + _typeStr + "' not recognised";
                result["error"] = err;
                return result;
            }


            Field f{_name, _type.value(), _required, _primaryKey, _system};
            new_fields.push_back(f);
        }

        if (type == "auth")
        {
            AuthTable auth;
            auth.name = name;
            auth.system = false;

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
            view.name = name;
            view.system = false;
            view.sourceSQL = _sourceSQL;

            if (_sourceSQL.empty())
            {
                json err;
                err["error"] = "View SQL Query is empty";
                result["error"] = err;
                return result;
            }

            schema_str = view.to_json().dump();
            table_ddl = view.to_sql();
        }
        else
        {
            BaseTable base;
            base.name = name;
            base.system = false;

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

        // Insert to __tables
        *sql <<
            "INSERT INTO __tables (id, name, type, schema, created, updated) VALUES (:id, :name, :type, :schema, :created, :updated)"
            ,
            soci::use(id), soci::use(name), soci::use(type),
            soci::use(schema_str), soci::use(*created_tm), soci::use(*created_tm);

        // Create actual SQL table
        *sql << table_ddl;


        json obj{entity};
        obj["id"] = id;
        // obj["created"] = created_tm;
        // obj["created"] = created_tm;
        result["data"] = obj;

        tr.commit();

        return result;
    }

    std::optional<json> TablesCrud::read(const std::string& id, const json& opts)
    {
        const auto sql = m_app->db().session();

        soci::row r;
        *sql << "SELECT id, name, type, schema FROM __tables WHERE id = :id", soci::use(id), soci::into(r);

        if (!sql->got_data())
        {
            return nullopt;
        }

        // const auto id = r.get<std::string>(0);
        const auto name = r.get<std::string>(1);
        const auto type = r.get<std::string>(2);
        const auto schema_json = r.get<std::string>(3);
        const json j = json::parse(schema_json);

        json tb;
        tb["id"] = id;
        tb["name"] = name;
        tb["type"] = type;
        tb["schema"] = j;

        return tb;
    }

    json TablesCrud::update(const std::string& id, const json& entity, const json& opts)
    {
        const auto sql = m_app->db().session();
        soci::transaction tr(*sql);
        json response;
        response["data"] = json{};
        response["error"] = json{};

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

    bool TablesCrud::remove(const std::string& id, const json& opts)
    {
        const auto sql = m_app->db().session();
        soci::transaction tr(*sql);

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

        return true;
    }

    std::vector<json> TablesCrud::list(const json& opts)
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
            // const auto has_api = row.get<bool>(4);

            // const json j = json::parse(schema_json);

            json tb;
            tb["id"] = id;
            tb["name"] = name;
            tb["type"] = type;
            // tb["has_api"] = has_api;
            // tb["schema"] = j;

            response.push_back(tb);
        }

        return response;
    }
} // mantis
