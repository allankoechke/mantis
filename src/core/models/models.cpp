//
// Created by allan on 09/05/2025.
//

#include "../../../include/mantis/mantis.h"
#include "soci/sqlite3/soci-sqlite3.h"

mantis::Validator::Validator()
{
    m_validators.clear();
    m_validators["email"] = json{
        {"regex", R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)"},
        {"error", "Email format is not valid"}
    };

    m_validators["password"] = json{
        {"regex", R"(^\S{8,}$)"},
        {"error", "Expected 8 chars minimum with no whitespaces."}
    };

    m_validators["password-long"] = json{
        {"regex", R"(^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)(?=.*[\W_]).{8,}$)"},
        {"error", "Expected at least one lowercase, uppercase, digit, special character, and a min 8 chars."}
    };
}

std::optional<json> mantis::Validator::find(const std::string& key)
{
    if (const auto it = m_validators.find(key); it != m_validators.end())
    {
        return it->second;
    }

    return std::nullopt;
}

mantis::json mantis::Validator::validate(const std::string& key, const std::string& value)
{
    json response{{"error", ""}, {"validated", false}};

    if (trim(key).empty())
    {
        response["error"] = "Validator key can't be empty!";
        return response;
    }

    const auto v = find(key).value_or(json::object());
    if (v.empty())
    {
        response["error"] = "Validator key is not available!";
        return response;
    }

    // Since we have a regex string, lets validate it and return if it fails ...
    const auto& reg = v["regex"].get<std::string>();
    const auto& err = v["error"].get<std::string>();

    if (const std::regex r_pattern(reg); !std::regex_match(value, r_pattern))
    {
        response["error"] = err;
        return response;
    }

    // If we reach here, then, all was validated correctly!
    response["error"] = "";
    response["validated"] = true;
    return response;
}

std::optional<mantis::FieldType> mantis::getFieldType(const std::string& fieldName)
{
    if (fieldName == "xml") return FieldType::XML;
    if (fieldName == "string") return FieldType::STRING;
    if (fieldName == "double") return FieldType::DOUBLE;
    if (fieldName == "date") return FieldType::DATE;
    if (fieldName == "int8") return FieldType::INT8;
    if (fieldName == "uint8") return FieldType::UINT8;
    if (fieldName == "int16") return FieldType::INT16;
    if (fieldName == "uint16") return FieldType::UINT16;
    if (fieldName == "int32") return FieldType::INT32;
    if (fieldName == "uint32") return FieldType::UINT32;
    if (fieldName == "int64") return FieldType::INT64;
    if (fieldName == "uint64") return FieldType::UINT64;
    if (fieldName == "blob") return FieldType::BLOB;
    if (fieldName == "json") return FieldType::JSON;
    if (fieldName == "bool") return FieldType::BOOL;
    if (fieldName == "file") return FieldType::FILE;
    if (fieldName == "files") return FieldType::FILES;
    return std::nullopt;
}

bool mantis::fieldExists(const TableType& type, const std::string& fieldName)
{
    // Find if field exists ...
    const auto contains = [&](const std::vector<std::string>& vec, const std::string& field_name) -> bool
    {
        return std::ranges::find(vec, field_name) != vec.end();
    };

    switch (type)
    {
    case TableType::View: return false;
    case TableType::Base:
        {
            return contains(baseFields, fieldName);
        }
    case TableType::Auth:
        {
            return contains(authFields, fieldName);
        }
    default: return false;
    }
}

bool mantis::isValidFieldType(const std::string& fieldType)
{
    // Valid field types for our backend
    std::vector<std::string> fieldTypes{
        "xml", "string", "double", "date",
        "int8", "uint8", "int16", "uint16", "int32", "uint32", "int64", "uint64",
        "blob", "json", "bool", "file", "files"
    };

    const auto it = std::ranges::find(fieldTypes, fieldType);
    return it != fieldTypes.end();
}

mantis::Field::Field(std::string n, const FieldType t, const bool req, const bool pk, const bool sys, json opts)
    : name(std::move(n)), type(t), required(req), primaryKey(pk), system(sys)
{
    if (!opts.empty())
    {
        if (opts.contains("validator"))
        {
            // Log::trace("Validator ...? {}", opts["validator"].is_null());
            if (opts["validator"].is_null())
                validator = std::nullopt;
            else
                validator = opts["validator"];
        }

        if (opts.contains("unique"))
        {
            // Log::trace("Unique ...? {}", opts["unique"].is_null());
            if (opts["unique"].is_null())
                isUnique = false;
            else
                isUnique = opts["unique"];
        }

        if (opts.contains("defaultValue"))
        {
            // Log::trace("Default Value ...? {}", opts["defaultValue"].is_null());
            if (opts["defaultValue"].is_null())
                defaultValue = std::nullopt;
            else
                defaultValue = opts.at("defaultValue").get<std::string>();
        }

        if (opts.contains("minValue"))
        {
            // Log::trace("Min Value ...? {}", opts["minValue"].is_null());
            if (opts["minValue"].is_null())
                minValue = std::nullopt;
            else
                minValue = opts.at("minValue").get<double>();
        }

        if (opts.contains("maxValue"))
        {
            // Log::trace("Max Value ...? {}", opts["maxValue"].is_null());
            if (opts["maxValue"].is_null())
                maxValue = std::nullopt;
            else
                maxValue = opts.at("maxValue").get<double>();
        }

        if (opts.contains("autoGeneratePattern"))
        {
            // Log::trace("Auto-Generate Pattern ...? {}", opts["autoGeneratePattern"].is_null());
            if (opts["autoGeneratePattern"].is_null())
                autoGeneratePattern = std::nullopt;
            else
                autoGeneratePattern = opts.at("autoGeneratePattern").get<std::string>();
        }
    }
}

json mantis::Field::to_json() const
{
    return {
        {"name", name},
        {"type", type},
        {"required", required},
        {"primaryKey", primaryKey},
        {"system", system},
        {"unique", isUnique},
        {"validator", validator},
        {"defaultValue", defaultValue},
        {"minValue", minValue},
        {"maxValue", maxValue},
        {"autoGeneratePattern", autoGeneratePattern}
    };
}

soci::db_type mantis::Field::toSociType() const
{
    return Field::toSociType(type);
}

soci::db_type mantis::Field::toSociType(const FieldType& f_type)
{
    if (f_type == FieldType::XML) return soci::db_xml;
    if (f_type == FieldType::DOUBLE) return soci::db_double;
    if (f_type == FieldType::DATE) return soci::db_date;
    if (f_type == FieldType::INT8) return soci::db_int8;
    if (f_type == FieldType::UINT8) return soci::db_uint8;
    if (f_type == FieldType::INT16) return soci::db_int16;
    if (f_type == FieldType::UINT16) return soci::db_uint16;
    if (f_type == FieldType::INT32) return soci::db_int32;
    if (f_type == FieldType::UINT32) return soci::db_uint32;
    if (f_type == FieldType::INT64) return soci::db_int64;
    if (f_type == FieldType::UINT64) return soci::db_uint64;
    if (f_type == FieldType::BLOB) return soci::db_blob;
    if (f_type == FieldType::BOOL) return soci::db_int8;
    if (f_type == FieldType::STRING || f_type == FieldType::JSON
        || f_type == FieldType::FILE || f_type == FieldType::FILES)
        return soci::db_string;

    throw std::invalid_argument("Unknown field type");
}

json mantis::Table::to_json() const
{
    json j;
    j["id"] = id;
    j["name"] = name;
    j["type"] = type;
    j["system"] = system;
    j["fields"] = json::array();
    j["has_api"] = has_api;

    for (const auto& f : fields) j["fields"].push_back(f.to_json());

    j["listRule"] = listRule;
    j["getRule"] = getRule;
    j["addRule"] = addRule;
    j["updateRule"] = updateRule;
    j["deleteRule"] = deleteRule;

    return j;
}

std::string mantis::Table::to_sql() const
{
    // Lambda function to handle any special case data type handling, this is for
    // generation of the SQL statement only not db data conversion.
    auto get_field_type = [](const FieldType& t, const std::shared_ptr<soci::session>& sql) -> std::string
    {
        const auto db_type = sql->get_backend()->get_backend_name();
        // For date types, enforce `text` type SQLite ONLY
        if (db_type == "sqlite3" && t == FieldType::DATE)
        {
            return "text";
        }

        // PostgreSQL has no support for `db_uint` and `db_uint8` so lets store them in `db_uint16`/`db_int16 types`
        if (db_type == "postgresql" && (t == FieldType::UINT8 || t == FieldType::INT8))
        {
            return t == FieldType::UINT8
                       ? sql->get_backend()->create_column_type(soci::db_uint16, 0, 0)
                       : sql->get_backend()->create_column_type(soci::db_int16, 0, 0);
        }

        // General catch for all other types
        return sql->get_backend()->create_column_type(Field::toSociType(t), 0, 0);
    };


    // Get DB Session
    const auto sql = MantisApp::instance().db().session();

    std::ostringstream ddl;
    ddl << "CREATE TABLE IF NOT EXISTS " << name << " (";


    for (size_t i = 0; i < fields.size(); ++i)
    {
        if (i > 0) ddl << ", ";
        const auto field = fields[i];

        ddl << field.name << " "
            << get_field_type(field.type, sql);

        if (field.primaryKey) ddl << " PRIMARY KEY";
        if (field.required) ddl << " NOT NULL";
        if (field.isUnique) ddl << " UNIQUE";
        if (field.defaultValue.has_value()) ddl << " DEFAULT '" << field.defaultValue.value() << "'";
    }

    ddl << ");";
    return ddl.str();
}

mantis::BaseTable::BaseTable()
{
    type = TableType::Base;
    fields = {
        Field("id", FieldType::STRING, true, true, true),
        Field("created", FieldType::DATE, true, false, true),
        Field("updated", FieldType::DATE, true, false, true)
    };
}

mantis::AuthTable::AuthTable()
{
    type = TableType::Auth;
    fields = {
        Field("id", FieldType::STRING, true, true, true),
        Field("created", FieldType::DATE, true, false, true),
        Field("updated", FieldType::DATE, true, false, true),
        Field("email", FieldType::STRING, true, false, true,
              json{{"unique", true}, {"minValue", 5}, {"validator", "email"}}),
        Field("password", FieldType::STRING, true, false, true,
              json{{"minValue", 8}, {"validator", "password"}}),
        Field("name", FieldType::STRING, true, false, true,
              json{{"unique", false}, {"minValue", 2}})
    };
}

mantis::ViewTable::ViewTable()
{
    type = TableType::View;
}

mantis::SystemTable::SystemTable()
{
    system = true;
    type = TableType::Base;
    fields = {
        Field("id", FieldType::STRING, true, true, true),
        Field("created", FieldType::DATE, true, false, true),
        Field("updated", FieldType::DATE, true, false, true)
    };
}

mantis::AdminTable::AdminTable()
{
    system = true;
    type = TableType::Auth;
    fields = {
        Field("id", FieldType::STRING, true, true, true),
        Field("email", FieldType::STRING, true, false, true,
              json{{"unique", true}, {"minValue", 5}, {"validator", "email"}}),
        Field("password", FieldType::STRING, true, false, true,
              json{{"minValue", 8}, {"validator", "password"}}),
        Field("created", FieldType::DATE, true, false, true),
        Field("updated", FieldType::DATE, true, false, true)
    };
}
