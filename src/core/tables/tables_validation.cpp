//
// Created by allan on 07/06/2025.
//

#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../../include/mantis/utils/utils.h"

#define __file__ "core/tables/tables_validation.cpp"

namespace mantis
{
    std::optional<std::string> TableUnit::validateRequestBody(const json& body) const
    {
        // If the table type is of view type, check that the SQL is passed in ...
        if (m_tableType == "view")
        {
            const auto& [pass, err] = viewTypeSQLCheck(body);
            if (!pass) return err;
        }

        else // For `base` and `auth` types
        {
            // Create default base object
            for (const auto& field : m_fields)
            {
                const auto& name = field.value("name", "");

                // Skip system generated fields
                if (name == "id" || name == "created" || name == "updated") continue;


                { // REQUIRED CONSTRAINT CHECK
                    const auto& [pass, err] = requiredConstraintCheck(field, body);
                    if (!pass) return err;
                }

                { // MINIMUM CONSTRAINT CHECK
                    const auto& [pass, err] = minimumConstraintCheck(field, body);
                    if (!pass) return err;
                }

                { // MAXIMUM CONSTRAINT CHECK
                    const auto& [pass, err] = maximumConstraintCheck(field, body);
                    if (!pass) return err;
                }

                { // VALIDATOR CONSTRAINT CHECK
                    const auto& [pass, err] = validatorConstraintCheck(field, body);
                    if (!pass) return err;
                }
            }
        }

        // Return null option for no error cases
        return std::nullopt;
    }

    std::optional<std::string> TableUnit::validateUpdateRequestBody(const json& body) const
    {
        // If the table type is of view type, check that the SQL is passed in ...
        if (m_tableType == "view")
        {
            const auto& [pass, err] = viewTypeSQLCheck(body);
            if (!pass) return err;
        }

        else // For `auth` and `base` types
        {
            // Create default base object
            for (const auto& [key, val] : body.items())
            {
                const auto j = findFieldByKey(key);

                if (!j.has_value())
                {
                    return std::format("Unknown field named `{}`!", key);
                }

                const auto& field = j.value();
                const auto& name = field.value("name", "");

                // Skip system generated fields
                if (name == "id" || name == "created" || name == "updated") continue;

                { // REQUIRED CONSTRAINT CHECK
                    const auto& [pass, err] = requiredConstraintCheck(field, body);
                    if (!pass) return err;
                }

                { // MINIMUM CONSTRAINT CHECK
                    const auto& [pass, err] = minimumConstraintCheck(field, body);
                    if (!pass) return err;
                }

                { // MAXIMUM CONSTRAINT CHECK
                    const auto& [pass, err] = maximumConstraintCheck(field, body);
                    if (!pass) return err;
                }

                { // VALIDATOR CONSTRAINT CHECK
                    const auto& [pass, err] = validatorConstraintCheck(field, body);
                    if (!pass) return err;
                }
            }
        }

        return std::nullopt;
    }

    std::pair<bool, std::string> TableUnit::minimumConstraintCheck(const json& field, const json& entity)
    {
        if (!field["minValue"].is_null())
        {
            const auto& min_value = field["minValue"].get<double>();
            const auto& field_name = field["name"].get<std::string>();
            const auto& field_type = field["name"].get<std::string>();

            if (field_type == "string" && entity.value(field_name, "").size() < static_cast<size_t>(min_value))
            {
                return std::make_pair(false,
                    std::format("Minimum Constraint Failed: Char length for `{}` should be >= {}",
                    field_name, static_cast<int>(min_value)));
            }

            if (field_type == "double" || field_type == "int8" || field_type == "uint8" || field_type == "int16" ||
                field_type == "uint16" || field_type == "int32" || field_type == "uint32" || field_type == "int64" ||
                field_type == "uint64")
            {
                if (entity.at(field_name) < min_value)
                {
                   return std::pair(false,
                       std::format("Minimum Constraint Failed: Value for `{}` should be >= {}",
                       field_name, min_value));
                }
            }
        }

        return std::make_pair(true, "");
    }

    std::pair<bool, std::string> TableUnit::maximumConstraintCheck(const json& field, const json& entity)
    {
        if (!field["maxValue"].is_null())
        {
            const auto max_value = field["maxValue"].get<double>();
            const auto& field_name = field["name"].get<std::string>();
            const auto& field_type = field["name"].get<std::string>();

            if (field_type == "string" && entity.value(field_name, "").size() > static_cast<size_t>(max_value))
            {
                return std::make_pair(false,
                    std::format("Minimum Constraint Failed: Char length for `{}` should be >= {}",
                    field_name, static_cast<int>(max_value)));
            }

            if (field_type == "double" || field_type == "int8" || field_type == "uint8" || field_type == "int16" ||
                field_type == "uint16" || field_type == "int32" || field_type == "uint32" || field_type == "int64" ||
                field_type == "uint64")
            {
                if (entity.at(field_name) > max_value)
                {
                    return std::pair(false,
                        std::format("Maximum Constraint Failed: Value for `{}` should be <= {}",
                        field_name, max_value));
                }
            }
        }

        return std::make_pair(true, "");
    }

    std::pair<bool, std::string> TableUnit::requiredConstraintCheck(const json& field, const json& entity)
    {
        // Get the required flag and the field name
        const auto& required = field.value("required", false);
        const auto& field_name = field["name"].get<std::string>();

        if (required && entity[field_name].is_null())
        {
            return std::make_pair(false, std::format("Field `{}` is required", field_name));
        }

        return std::make_pair(true, "");
    }

    std::pair<bool, std::string> TableUnit::validatorConstraintCheck(const json& field, const json& entity)
    {
        if (!field["validator"].is_null())
        {
            const auto& pattern = field["validator"].get<std::string>();
            const auto& field_name = field["name"].get<std::string>();
            const auto& field_type = field["type"].get<std::string>();

            // Check if we have a regex or typed validator from our store
            const auto opt =  MantisApp::instance().validators().find(pattern);
            if (opt.has_value() && field_type == "string")
            {
                // Since we have a regex string, lets validate it and return if it fails ...
                const auto& reg = opt.value()["regex"].get<std::string>();
                const auto& err = opt.value()["error"].get<std::string>();

                auto f = entity.at(field_name).get<std::string>();
                if (const std::regex r_pattern(reg); !std::regex_match(f, r_pattern))
                {
                    return std::make_pair(false, err);
                }
            }
        }

        return std::make_pair(true, "");
    }

    std::pair<bool, std::string> TableUnit::viewTypeSQLCheck(const json& entity)
    {
        if (!entity.contains("sql") || entity["sql"].is_null() || trim(entity["sql"].get<std::string>()).empty())
        {
            return std::make_pair(false, "View tables require a valid SQL View query!");
        }

        // const auto& sql = trim(entity.at("sql").get<std::string>());
        // Check that it contains an `id` field
        // Check it does not contain malicious things!!

        return std::make_pair(true, "");
    }
}
