//
// Created by allan on 07/06/2025.
//

#include "../../include/mantis/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../include/mantis/utils.h"

namespace mantis
{
    std::optional<json> TableUnit::validateRequestBody(const json& body) const
    {
        // Create default base object
        for (const auto& field : m_fields)
        {
            json obj;
            const auto& name = field.value("name", "");

            // Skip system generated fields
            if (name == "id" || name == "created" || name == "updated") continue;

            const auto& type = field.value("type", "");
            // const auto& value = getTypedValue(body, name, type);

            // || body.at(name).is_null()
            // TODO current assumption is that the value is not empty, fix that later
            if (const auto& required = field.value("required", false);
                required && !body.contains(name))
            {
                obj["error"] = "Field '" + name + "' is required";
                obj["status"] = 400;
                return obj;
            }

            Log::trace("Check for min value");
            if (!field["minValue"].is_null())
            {
                const auto minValue = field["minValue"].get<double>();
                Log::trace("Checking if minValue is satisfied: Is String? {}, Value? {}, Condition: {}",
                           type == "string", body.value(name, "").size(),
                           body.value(name, "").size() < static_cast<int>(minValue));
                if (type == "string" && body.value(name, "").size() < static_cast<int>(minValue))
                {
                    obj["error"] = name + " should be at least " + std::to_string(static_cast<int>(minValue)) +
                        " chars long.";
                    obj["status"] = 400;
                    return obj;
                }

                Log::trace("Min Value check for integral types ...");
                auto checkMinValueFunc = [&](auto _val, auto _min)
                {
                    if (_val < _min)
                    {
                        obj["error"] = "Field '" + name + "' should be greater or equal to " + std::to_string(_min);
                        obj["status"] = 400;
                        return true;
                    }
                    return false;
                };

                if (type == "double" &&
                    checkMinValueFunc(body.at(name).get<double>(), minValue))
                {
                    return obj;
                }
                else if (type == "int8" &&
                    checkMinValueFunc(body.at(name).get<int8_t>(), static_cast<int8_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "uint8" &&
                    checkMinValueFunc(body.at(name).get<uint8_t>(), static_cast<uint8_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "int16" &&
                    checkMinValueFunc(body.at(name).get<int16_t>(), static_cast<int16_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "uint16" &&
                    checkMinValueFunc(body.at(name).get<uint16_t>(), static_cast<uint16_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "int32" &&
                    checkMinValueFunc(body.at(name).get<int32_t>(), static_cast<int32_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "uint32" &&
                    checkMinValueFunc(body.at(name).get<uint32_t>(), static_cast<uint32_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "int64" &&
                    checkMinValueFunc(body.at(name).get<int64_t>(), static_cast<int64_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "uint64" &&
                    checkMinValueFunc(body.at(name).get<uint64_t>(), static_cast<uint64_t>(minValue)))
                {
                    return obj;
                }
            }

            Log::trace("Check for max value");
            if (!field["maxValue"].is_null())
            {
                const auto maxValue = field["maxValue"].get<double>();

                if (type == "string" && body.at(name).size() > maxValue)
                {
                    obj["error"] = "String value should be at most " + std::to_string(maxValue) + " characters long.";
                    obj["status"] = 400;
                    return obj;
                }

                if (type == "double"
                    || type == "int8" || type == "uint8"
                    || type == "int16" || type == "uint16"
                    || type == "int32" || type == "uint32"
                    || type == "int64" || type == "uint64")
                {
                    if (body.at(name) > maxValue)
                    {
                        obj["error"] = "Field '" + name + "' value should be less or equal to " + std::to_string(
                            maxValue);
                        obj["status"] = 400;
                        return obj;
                    }
                }
            }

            // if (field["defaultValue"] && !body.contains(name))
            // {
            //
            // }

            Log::trace("Check for view type");
            // If the table type is of view type, check that the SQL is passed in ...
            if (m_tableType == "view")
            {
                auto sql_stmt = body.at("sql").get<std::string>();
                trim(sql_stmt);
                if (sql_stmt.empty())
                {
                    obj["error"] = "View tables require SQL query Statement";
                    obj["status"] = 400;
                    return obj;
                }
            }
        }

        Log::trace("Done checking");
        return nullopt;
    }

    std::optional<json> TableUnit::validateUpdateRequestBody(const json& body) const
    {
        // Create default base object
        for (const auto& [key, val] : body.items())
        {
            json obj;
            const auto j = findFieldByKey(key);

            if (!j.has_value())
            {
                obj["error"] = "Field '" + key + "' is required";
                obj["status"] = 400;
                return obj;
            }

            const auto& field = j.value();
            const auto& name = field.value("name", "");

            // Skip system generated fields
            if (name == "id" || name == "created" || name == "updated") continue;

            const auto& type = field.value("type", "");

            if (const auto& required = field.value("required", false);
                required && !body.contains(name))
            {
                obj["error"] = "Field '" + name + "' is required";
                obj["status"] = 400;
                return obj;
            }

            Log::trace("Is minValue set? {}", field["minValue"].is_null());
            if (!field["minValue"].is_null())
            {
                const auto minValue = field["minValue"].get<double>();

                Log::trace("Checking if minValue is satisfied: Is String? {}, Value? {}, Condition: {}",
                           type == "string", body.value(name, "").size(),
                           body.value(name, "").size() < static_cast<int>(minValue));
                if (type == "string" && body.value(name, "").size() < static_cast<int>(minValue))
                {
                    obj["error"] = "String value should be at least " + std::to_string(minValue) + " characters long.";
                    obj["status"] = 400;
                    return obj;
                }

                Log::trace("Min Value check for integral types ...");
                auto checkMinValueFunc = [&](auto _val, auto _min)
                {
                    if (_val < _min)
                    {
                        obj["error"] = "Field '" + name + "' should be greater or equal to " + std::to_string(_min);
                        obj["status"] = 400;
                        return true;
                    }
                    return false;
                };

                if (type == "double" &&
                    checkMinValueFunc(body.at(name).get<double>(), minValue))
                {
                    return obj;
                }
                else if (type == "int8" &&
                    checkMinValueFunc(body.at(name).get<int8_t>(), static_cast<int8_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "uint8" &&
                    checkMinValueFunc(body.at(name).get<uint8_t>(), static_cast<uint8_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "int16" &&
                    checkMinValueFunc(body.at(name).get<int16_t>(), static_cast<int16_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "uint16" &&
                    checkMinValueFunc(body.at(name).get<uint16_t>(), static_cast<uint16_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "int32" &&
                    checkMinValueFunc(body.at(name).get<int32_t>(), static_cast<int32_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "uint32" &&
                    checkMinValueFunc(body.at(name).get<uint32_t>(), static_cast<uint32_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "int64" &&
                    checkMinValueFunc(body.at(name).get<int64_t>(), static_cast<int64_t>(minValue)))
                {
                    return obj;
                }
                else if (type == "uint64" &&
                    checkMinValueFunc(body.at(name).get<uint64_t>(), static_cast<uint64_t>(minValue)))
                {
                    return obj;
                }
            }

            // Log::trace("Check for max value");
            if (!field["maxValue"].is_null())
            {
                const auto maxValue = field["maxValue"].get<double>();

                if (type == "string" && body.value(name, "").size() > maxValue)
                {
                    obj["error"] = "String value should be at most " + std::to_string(maxValue) + " characters long.";
                    obj["status"] = 400;
                    return obj;
                }

                if (type == "double"
                    || type == "int8" || type == "uint8"
                    || type == "int16" || type == "uint16"
                    || type == "int32" || type == "uint32"
                    || type == "int64" || type == "uint64")
                {
                    if (body.at(name) > maxValue)
                    {
                        obj["error"] = "Field '" + name + "' value should be less or equal to " + std::to_string(
                            maxValue);
                        obj["status"] = 400;
                        return obj;
                    }
                }
            }

            if (!field["validator"].is_null())
            {
                auto pattern = field["validator"].get<std::string>();
                if (const auto opt = m_app->validators().find(pattern);
                    opt.has_value() && type == "string")
                {
                    // Since we have a regex string, lets validate it and return if it fails ...
                    const auto& reg = opt.value()["regex"].get<std::string>();
                    const auto& err = opt.value()["error"].get<std::string>();

                    auto f = body.at(name).get<std::string>();
                    if (const std::regex r_pattern(reg); !std::regex_match(f, r_pattern))
                    {
                        obj["error"] = err;
                        obj["status"] = 400;
                        return obj;
                    }
                }
            }

            // if (field["defaultValue"] && !body.contains(name))
            // {
            //
            // }

            // Log::trace("Check for view type");
            // If the table type is of view type, check that the SQL is passed in ...
            if (m_tableType == "view")
            {
                auto sql_stmt = body.at("sql").get<std::string>();
                trim(sql_stmt);
                if (sql_stmt.empty())
                {
                    obj["error"] = "View tables require SQL query Statement";
                    obj["status"] = 400;
                    return obj;
                }
            }
        }

        // Log::trace("Done checking");
        return nullopt;
    }
}
