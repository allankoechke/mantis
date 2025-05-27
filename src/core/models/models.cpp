//
// Created by allan on 09/05/2025.
//

#include "../../../include/mantis/mantis.h"

std::optional<mantis::FieldType> mantis::getFieldType(const std::string& fieldName)
{
    if ( fieldName == "url") return FieldType::Url;
    if ( fieldName == "text") return FieldType::Text;
    if ( fieldName == "integer") return FieldType::Integer;
    if ( fieldName == "float") return FieldType::Float;
    if ( fieldName == "boolean") return FieldType::Boolean;
    if ( fieldName == "datetime") return FieldType::DateTime;
    if ( fieldName == "email") return FieldType::Email;
    if ( fieldName == "password") return FieldType::Password;
    if ( fieldName ==  "json") return FieldType::JSON;
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

json mantis::Rule::to_json() const
{
    return json{{"expression", expression}, {"enabled", enabled}};
}

mantis::Field::Field(std::string n, const FieldType t, const bool req, const bool pk, const bool sys)
    : name(std::move(n)), type(t), required(req), primaryKey(pk), system(sys)
{}

json mantis::Field::to_json() const
{
    return {
        { "name", name },
        { "type", type },
        { "required", required },
        { "primaryKey", primaryKey },
        { "system", system },
        { "defaultValue", defaultValue },
        { "minValue", minValue },
        { "maxValue", maxValue },
        { "autoGeneratePattern", autoGeneratePattern }
    };
}

std::string mantis::Field::to_sql() const
{
    std::string sql_type;
    switch (type) {
    case FieldType::Url:
    case FieldType::Text:
    case FieldType::Email:      sql_type = "TEXT";      break;
    case FieldType::Integer:    sql_type = "INTEGER";   break;
    case FieldType::Float:      sql_type = "REAL";      break;
    case FieldType::Boolean:    sql_type = "BOOLEAN";   break;
    case FieldType::DateTime:   sql_type = "DATETIME";  break;
    case FieldType::Password:   sql_type = "TEXT";      break;
    case FieldType::JSON:       sql_type = "JSON";      break;
    }

    std::string sql = name + " " + sql_type;

    if (primaryKey) sql += " PRIMARY KEY";
    if (required) sql += " NOT NULL";
    if (defaultValue.has_value()) sql += " DEFAULT '" + *defaultValue + "'";

    return sql;
}

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

    j["rules"] = {
        {"list", listRule.to_json()},
        {"get", getRule.to_json()},
        {"add", addRule.to_json()},
        {"update", updateRule.to_json()},
        {"delete", deleteRule.to_json()}
    };

    return j;
}

std::string mantis::Table::to_sql() const
{
    std::string sql = "CREATE TABLE IF NOT EXISTS " + name + " (";

    for (size_t i = 0; i < fields.size(); ++i) {
        sql += fields[i].to_sql();
        if (i < fields.size() - 1) sql += ", ";
    }
    sql += ");";

    return sql;

    // std::ostringstream ddl;
    // ddl << "CREATE TABLE " << tableName << " (";
    //
    // for (size_t i = 0; i < columns.size(); ++i) {
    //     if (i > 0) ddl << ", ";
    //     ddl << columns[i].first << " "
    //         << sql.get_backend()->create_column_type(columns[i].second, 0, 0);
    // }
    //
    // ddl << ")";
    // return ddl.str();
}

mantis::BaseTable::BaseTable()
{
    type = TableType::Base;
    fields = {
        Field("id", FieldType::Text, true, true, true),
        Field("created", FieldType::DateTime, true, false, true),
        Field("updated", FieldType::DateTime, true, false, true)
    };
}

mantis::AuthTable::AuthTable()
{
    type = TableType::Auth;
    fields = {
        Field("id", FieldType::Text, true, true, true),
        Field("created", FieldType::DateTime, true, false, true),
        Field("updated", FieldType::DateTime, true, false, true),
        Field("email", FieldType::Email, true, false, true),
        Field("password", FieldType::Password, true, false, true),
        Field("name", FieldType::Text)
    };
}

mantis::ViewTable::ViewTable()
{
    type = TableType::View;
}
