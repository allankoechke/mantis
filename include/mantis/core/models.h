//
// Created by allan on 08/05/2025.
//

#ifndef MODELS_H
#define MODELS_H

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace Mantis
{
    // Enum of the table type created,
    // base table types provide `index`, `created`, `updated`
    // auth table type provide `base` type + `email`, `password`, `name`
    // view table type provide readonly `sql`
    typedef enum TableType
    {
        BaseTable = 1,
        AuthTable,
        ViewTable
    } TableType;

    NLOHMANN_JSON_SERIALIZE_ENUM(TableType, {
        {TableType::BaseTable, "base"},
        {TableType::AuthTable, "auth"},
        {TableType::ViewTable, "view"}
    })

    // Text, Number, Bool, etc.
    // Enum to represent different data types for fields
    enum class FieldType {
        Url,
        Text,
        Integer,
        Float,
        Boolean,
        DateTime,
        Email,
        Password,
        JSON
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(FieldType, {
        { FieldType::Url,       "url" },
        { FieldType::Text,      "text" },
        { FieldType::Integer,   "integer" },
        { FieldType::Float,     "float" },
        { FieldType::Boolean,   "boolean" },
        { FieldType::DateTime,  "datetime" },
        { FieldType::Email,     "email" },
        { FieldType::Password,  "password" },
        { FieldType::JSON,      "json" }
    })

    // Access Rule Struct
    typedef struct Rule
    {
        std::string expression;
        bool enabled;

        json to_json() const;
    } Rule;

    // Field definition
    struct Field {
        std::string name;
        FieldType type;

        bool required = false;
        bool primaryKey = false;
        bool system = false;

        std::optional<std::string> defaultValue;     // as string, parse based on type
        std::optional<int> minValue;
        std::optional<int> maxValue;
        std::optional<std::string> autoGeneratePattern; // regex for auto-gen strings

        Field() = default;

        // Convenience constructor
        Field(std::string n, const FieldType t, const bool req = false, const bool pk = false, const bool sys = false);

        [[nodiscard]] json to_json() const;
        [[nodiscard]] std::string to_sql() const;
    };

    // Represents a generic table in the system
    struct Table {
        std::string id;
        std::string name;
        TableType type;
        std::string schema;

        std::vector<Field> fields;

        Rule listRule;
        Rule getRule;
        Rule addRule;
        Rule updateRule;
        Rule deleteRule;

        virtual json to_json() const;

        virtual std::string to_sql() const;
    };

    // Specific model for Base table (user-defined)
    struct BaseTable : Table {
        bool enableSync = true;

        BaseTable();
    };

    // Specific model for Auth table
    struct AuthTable : Table {
        std::string usernameField = "email";
        std::string passwordField = "password";
        bool enableSync = true;

        AuthTable();
    };

    // Specific model for View table
    struct ViewTable : Table {
        std::string sourceSQL;
        bool enableSync = false;

        ViewTable();
    };
}

#endif //MODELS_H
