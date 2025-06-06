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

    std::cout << "VALIDATOR INIT()\n";
}

std::optional<json> mantis::Validator::find(const std::string& key)
{
    if (const auto it = m_validators.find(key); it != m_validators.end())
    {
        return it->second;
    }

    return nullopt;
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
    return std::nullopt;
}

bool mantis::fieldExists(const TableType& type, const std::string& fieldName)
{
    // Find if field exists ...
    const auto contains = [&](const std::vector<std::string>& vec, const std::string& fieldName) -> bool
    {
        return std::find(vec.begin(), vec.end(), fieldName) != vec.end();
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

mantis::Field::Field(std::string n, const FieldType t, const bool req, const bool pk, const bool sys, json opts)
    : name(std::move(n)), type(t), required(req), primaryKey(pk), system(sys)
{
    if (!opts.empty())
    {
        if (opts.contains("validator"))
        {
            validator = opts.at("validator").get<std::string>();
        }

        if (opts.contains("unique"))
        {
            isUnique = opts.at("unique").get<bool>();
        }

        if (opts.contains("defaultValue"))
        {
            // Handle different types ...
            // TODO This may/will throw an error, check on that
            defaultValue = opts.at("defaultValue").get<std::string>();
        }

        if (opts.contains("minValue"))
        {
            minValue = opts.at("minValue").get<double>();
        }

        if (opts.contains("maxValue"))
        {
            maxValue = opts.at("maxValue").get<double>();
        }

        if (opts.contains("autoGeneratePattern"))
        {
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
    if (type == FieldType::XML) return soci::db_xml;
    if (type == FieldType::STRING) return soci::db_string;
    if (type == FieldType::DOUBLE) return soci::db_double;
    if (type == FieldType::DATE) return soci::db_date;
    if (type == FieldType::INT8) return soci::db_int8;
    if (type == FieldType::UINT8) return soci::db_uint8;
    if (type == FieldType::INT16) return soci::db_int16;
    if (type == FieldType::UINT16) return soci::db_uint16;
    if (type == FieldType::INT32) return soci::db_int32;
    if (type == FieldType::UINT32) return soci::db_uint32;
    if (type == FieldType::INT64) return soci::db_int64;
    if (type == FieldType::UINT64) return soci::db_uint64;
    if (type == FieldType::BLOB) return soci::db_blob;
    if (type == FieldType::JSON) return soci::db_string;
    if (type == FieldType::BOOL) return soci::db_int8;
    return soci::db_string;
}

mantis::Table::Table(MantisApp* app): m_app(app) {}

json mantis::Table::to_json() const
{
    json j;
    j["id"] = id;
    j["name"] = name;
    j["type"] = type;
    j["schema"] = schema;
    j["system"] = system;
    j["fields"] = json::array();

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
    // Get DB Session
    const auto sql = m_app->db().session();

    std::ostringstream ddl;
    ddl << "CREATE TABLE IF NOT EXISTS " << name << " (";

    for (size_t i = 0; i < fields.size(); ++i)
    {
        if (i > 0) ddl << ", ";
        const auto field = fields[i];
        ddl << field.name << " "
            << (field.type == FieldType::DATE
                    ? (sql->get_backend()->get_backend_name() == "sqlite3"
                           ? "text"
                           : sql->get_backend()->create_column_type(field.toSociType(), 0, 0))
                    : sql->get_backend()->create_column_type(field.toSociType(), 0, 0));

        if (field.primaryKey) ddl << " PRIMARY KEY";
        if (field.required) ddl << " NOT NULL";
        if (field.isUnique) ddl << " UNIQUE";
        if (field.defaultValue.has_value()) ddl << " DEFAULT '" << field.defaultValue.value() << "'";
    }

    ddl << ");";
    return ddl.str();
}

mantis::BaseTable::BaseTable(MantisApp* app): Table(app)
{
    type = TableType::Base;
    fields = {
        Field("id", FieldType::STRING, true, true, true),
        Field("created", FieldType::DATE, true, false, true),
        Field("updated", FieldType::DATE, true, false, true)
    };
}

mantis::AuthTable::AuthTable(MantisApp* app): Table(app)
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
              json{{"unique", true}, {"minValue", 6}})
    };
}

mantis::ViewTable::ViewTable(MantisApp* app): Table(app)
{
    type = TableType::View;
}

mantis::SystemTable::SystemTable(MantisApp* app): BaseTable(app)
{
    system = true;
    type = TableType::Base;
    fields = {
        Field("id", FieldType::STRING, true, true, true),
        Field("created", FieldType::DATE, true, false, true),
        Field("updated", FieldType::DATE, true, false, true)
    };
}

mantis::AdminTable::AdminTable(MantisApp* app): AuthTable(app)
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
