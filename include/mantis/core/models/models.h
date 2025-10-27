//
// Created by allan on 08/05/2025.
//

#ifndef MODELS_H
#define MODELS_H

#include <optional>
#include <soci/soci.h>
#include <nlohmann/json.hpp>
#include "../../utils/utils.h"

namespace mantis
{
    using json = nlohmann::json;

    class MantisApp;

    class Validator
    {
        std::unordered_map<std::string, json> m_validators;

    public:
        Validator();

        std::optional<json> find(const std::string& key);

        json validate(const std::string& key, const std::string& value);
    };


    // Enum of the table type created,
    // base table types provide `index`, `created`, `updated`
    // auth table type provide `base` type + `email`, `password`, `name`
    // view table type provide readonly `sql`
    typedef enum class TableType
    {
        Base = 1,
        Auth,
        View
    } TableType;

    NLOHMANN_JSON_SERIALIZE_ENUM(TableType, {
                                 {TableType::Base, "base"},
                                 {TableType::Auth, "auth"},
                                 {TableType::View, "view"}
                                 })

    typedef enum class FieldTypeDecl
    {
        XML = soci::db_xml,
        STRING = soci::db_string,
        DOUBLE = soci::db_double,
        DATE = soci::db_date,
        INT8 = soci::db_int8,
        UINT8 = soci::db_uint8,
        INT16 = soci::db_int16,
        UINT16 = soci::db_uint16,
        INT32 = soci::db_int32,
        UINT32 = soci::db_uint32,
        INT64 = soci::db_int64,
        UINT64 = soci::db_uint64,
        BLOB = soci::db_blob,
        // User defined types
        JSON,
        BOOL,
        FILE, // Hold file name
        FILES // Hold an array of file
    } FieldType;

    NLOHMANN_JSON_SERIALIZE_ENUM(FieldType, {
                                 { FieldType::XML, "xml" },
                                 { FieldType::STRING, "string" },
                                 { FieldType::DOUBLE, "double" },
                                 { FieldType::DATE, "date" },
                                 { FieldType::INT8, "int8" },
                                 { FieldType::UINT8, "uint8" },
                                 { FieldType::INT16, "int16" },
                                 { FieldType::UINT16, "uint16" },
                                 { FieldType::INT32, "int32" },
                                 { FieldType::UINT32, "uint32" },
                                 { FieldType::INT64, "int64" },
                                 { FieldType::UINT64, "uint64" },
                                 { FieldType::BLOB, "blob" },
                                 { FieldType::JSON, "json" },
                                 { FieldType::BOOL, "bool" },
                                 { FieldType::FILE, "file" },
                                 { FieldType::FILES, "files" },
                                 })

    const std::vector<std::string> baseFields = {"id", "created", "updated"};
    const std::vector<std::string> authFields = {"id", "created", "updated", "name", "email", "password"};

    std::optional<FieldType> getFieldType(const std::string& fieldName);

    bool fieldExists(const TableType& type, const std::string& fieldName);

    bool isValidFieldType(const std::string& fieldType);

    // Access rule expression
    typedef std::string Rule;

    // Field definition
    struct Field
    {
        std::string name;
        FieldType type;

        bool required = false;
        bool primaryKey = false;
        bool system = false;

        std::optional<std::string> defaultValue; // as string, parse based on type
        std::optional<std::string> regexPattern;
        std::optional<double> minValue;
        std::optional<double> maxValue;
        bool isUnique = false;
        std::optional<std::string> validator;
        std::optional<std::string> autoGeneratePattern; // regex for auto-gen strings

        // Convenience constructor
        Field(std::string n, FieldType t, bool req = false, bool pk = false, bool sys = false,
              json opts = json::object());

        [[nodiscard]]
        json to_json() const;

        [[nodiscard]]
        soci::db_type toSociType() const;

        [[nodiscard]]
        static soci::db_type toSociType(const FieldType& f_type);
    };

    // Represents a generic table in the system
    struct Table
    {
        virtual ~Table() = default;
        std::string id;
        std::string name;
        TableType type;
        bool system = false;
        bool has_api = true;

        std::vector<Field> fields;

        Rule listRule;
        Rule getRule;
        Rule addRule;
        Rule updateRule;
        Rule deleteRule;

        // Constructor
        Table() = default;

        [[nodiscard]]
        virtual json to_json() const;

        [[nodiscard]]
        virtual std::string to_sql() const;
    };

    // Specific model for Base table (user-defined)
    struct BaseTable : Table
    {
        bool enableSync = true;

        BaseTable();
    };

    // Specific model for Auth table
    struct AuthTable : Table
    {
        std::string usernameField = "email";
        std::string passwordField = "password";
        bool enableSync = true;

        // Member Functions
        AuthTable();
        ~AuthTable() override = default;
    };

    // Specific model for View table
    struct ViewTable : Table
    {
        std::string sourceSQL;
        bool enableSync = false;

        ViewTable();
    };

    struct SystemTable final : BaseTable
    {
        bool enableSync = true;

        SystemTable();
        ~SystemTable() override = default;
    };

    struct AdminTable final : AuthTable
    {
        bool enableSync = true;

        AdminTable();
    };
}

#endif //MODELS_H
