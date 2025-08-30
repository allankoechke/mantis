//
// Created by allan on 07/06/2025.
//

#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../../include/mantis/utils/utils.h"

#include <iomanip>

#define __file__ "core/tables/tables_utils.cpp"

namespace mantis
{
    std::optional<json> TableUnit::findFieldByKey(const std::string& key) const
    {
        if (key.empty()) return std::nullopt;

        for (auto field : m_fields)
        {
            if (field.value("name", "") == key) return field;
        }

        return std::nullopt;
    }

    json TableUnit::checkValueInColumns(const std::string& value, const std::vector<std::string>& columns) const
    {
        // default response object
        json res{{"error", ""}, {"data", json::object()}};

        // Get a session object
        const auto sql = MantisApp::instance().db().session();

        try
        {
            // Build dynamic WHERE clause
            std::string whereClause;
            for (size_t i = 0; i < columns.size(); ++i)
            {
                if (i > 0) whereClause += " OR ";
                whereClause += columns[i] + " = :value";
            }

            const std::string query = "SELECT * FROM " + m_tableName + " WHERE " + whereClause + " LIMIT 1";

            // Run query
            soci::row r;
            *sql << query, soci::use(value), soci::into(r);

            const auto obj = parseDbRowToJson(r);
            res["data"] = obj;
        }
        catch (const soci::soci_error& e)
        {
            res["error"] = e.what();
        }
        catch (const std::exception& e)
        {
            res["error"] = e.what();
        }
        return res;
    }

    json TableUnit::parseDbRowToJson(const soci::row& row) const
    {
        // Use the internal m_fields for parsing fields
        return parseDbRowToJson(row, m_fields);
    }

    json TableUnit::parseDbRowToJson(const soci::row& row, const std::vector<json>& ref_fields) const
    {
        // Guard against empty reference schema fields
        if (ref_fields.empty())
            throw std::runtime_error(std::format("Parse db row error, empty reference schema fields passed!"));

        // Build response json object
        json j;
        for (size_t i = 0; i < row.size(); i++)
        {
            const auto colName = row.get_properties(i).get_name();
            const auto colType = getColTypeFromName(colName, ref_fields);

            // Check column type is valid type
            if (colType.empty() || !isValidFieldType(colType)) // Or not in expected types
            {
                // Throw an error for unknown types
                throw std::runtime_error(std::format("Unknown column type `{}` for column `{}`", colType, colName));
            }

            // Handle null values immediately
            if (row.get_indicator(i) == soci::i_null)
            {
                // Handle null value in JSON
                j[colName] = nullptr;
                continue;
            }

            // Handle type conversions
            if (colType == "xml" || colType == "string")
            {
                j[colName] = row.get<std::string>(i, "");
            }
            else if (colType == "double")
            {
                j[colName] = row.get<double>(i);
            }
            else if (colType == "date")
            {
                j[colName] = mantis::dbDateToString(MantisApp::instance().dbTypeByName(), row, i);
            }
            else if (colType == "int8")
            {
                j[colName] = row.get<int8_t>(i);
            }
            else if (colType == "uint8")
            {
                j[colName] = row.get<uint8_t>(i);
            }
            else if (colType == "int16")
            {
                j[colName] = row.get<int16_t>(i);
            }
            else if (colType == "uint16")
            {
                j[colName] = row.get<uint16_t>(i);
            }
            else if (colType == "int32")
            {
                j[colName] = row.get<int32_t>(i);
            }
            else if (colType == "uint32")
            {
                j[colName] = row.get<uint32_t>(i);
            }
            else if (colType == "int64")
            {
                j[colName] = row.get<int64_t>(i);
            }
            else if (colType == "uint64")
            {
                j[colName] = row.get<uint64_t>(i);
            }
            else if (colType == "blob")
            {
                // TODO ? How do we handle BLOB?
                // j[colName] = row.get<std::string>(i);
            }
            else if (colType == "json" || colType == "list")
            {
                j[colName] = row.get<json>(i);
            }
            else if (colType == "bool")
            {
                j[colName] = row.get<bool>(i);
            }
            else if (colType == "file")
            {
                j[colName] = row.get<std::string>(i);
            }
            else if (colType == "files")
            {
                j[colName] = row.get<json>(i);
            }
        }

        return j;
    }

    json TableUnit::getValueFromType(const std::string& type, const std::string& value)
    {
        json obj;
        const auto content = trim(value);
        if (content.empty())
        {
            obj["value"] = nullptr;
        }
        else if (type == "xml" || type == "string" || type == "date" || type == "file")
        {
            obj["value"] = content;
        }
        else if (type == "double" || type == "int8" || type == "uint8" || type == "int16" ||
            type == "uint16" || type == "int32" ||
            type == "uint32" || type == "int64" || type == "uint64")
        {
            obj["value"] = json::parse(content);
        }
        else if (type == "json" || type == "bool")
        {
            obj["value"] = json::parse(content);
        }
        else
        {
            obj["value"] = content;
        }

        return obj;
    }

    std::optional<json> TableUnit::bindEntityToSociValue(soci::values& vals, const json& entity) const
    {
        // Bind parameters dynamically
        for (const auto& field : m_fields)
        {
            const auto field_name = field.at("name").get<std::string>();

            if (field_name == "id" || field_name == "created" || field_name == "updated")
            {
                continue;
            }

            // For password types, let's hash them before binding to DB
            if (field_name == "password")
            {
                // Extract password value and hash it
                auto res = hashPassword(entity.value(field_name, ""));
                if (!res.value("error", "").empty())
                {
                    // Something went wrong while hashing password
                    Log::critical("Failed to hash user password. Reason: {}", res.value("error", ""));

                    json result;
                    result["error"] = res.value("error", "");
                    result["data"] = json::object();
                    result["status"] = 500;
                    return result;
                }

                // Add the hashed password to the soci::vals
                vals.set(field_name, res.at("hash").get<std::string>());
            }

            else
            {
                // Skip fields that are not in the json object
                if (!entity.contains(field_name)) continue;

                // If the value is null, set i_null and continue
                if (entity[field_name].is_null())
                {
                    std::optional<int> val; // Set to optional, no value is set in db
                    vals.set(field_name, val, soci::i_null);
                    continue;
                }

                // For non-null values, set the value accordingly
                const auto field_type = field.at("type").get<std::string>();
                if (field_type == "xml" || field_type == "string" || field_type == "file")
                {
                    vals.set(field_name, entity.value(field_name, ""));
                }

                else if (field_type == "double")
                {
                    vals.set(field_name, entity.value(field_name, 0.0));
                }

                else if (field_type == "date")
                {
                    auto dt_str = entity.value(field_name, "");
                    if (dt_str.empty())
                    {
                        vals.set(field_name, 0, soci::i_null);
                    }
                    else
                    {
                        std::tm tm{};
                        std::istringstream ss{dt_str};
                        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

                        vals.set(field_name, tm);
                    }
                }

                else if (field_type == "int8")
                {
                    vals.set(field_name, static_cast<int8_t>(entity.value(field_name, 0)));
                }

                else if (field_type == "uint8")
                {
                    vals.set(field_name, static_cast<uint8_t>(entity.value(field_name, 0)));
                }

                else if (field_type == "int16")
                {
                    vals.set(field_name, static_cast<int16_t>(entity.value(field_name, 0)));
                }

                else if (field_type == "uint16")
                {
                    vals.set(field_name, static_cast<uint16_t>(entity.value(field_name, 0)));
                }

                else if (field_type == "int32")
                {
                    vals.set(field_name, static_cast<int32_t>(entity.value(field_name, 0)));
                }

                else if (field_type == "uint32")
                {
                    vals.set(field_name, static_cast<uint32_t>(entity.value(field_name, 0)));
                }

                else if (field_type == "int64")
                {
                    vals.set(field_name, static_cast<int64_t>(entity.value(field_name, 0)));
                }

                else if (field_type == "uint64")
                {
                    vals.set(field_name, static_cast<uint64_t>(entity.value(field_name, 0)));
                }

                else if (field_type == "blob")
                {
                    // TODO implement BLOB type
                    // vals.set(field_name, entity.value(field_name, sql->empty_blob()));
                }

                else if (field_type == "json")
                {
                    vals.set(field_name, entity.value(field_name, json::object()));
                }

                else if (field_type == "bool")
                {
                    vals.set(field_name, entity.value(field_name, false));
                }

                else if (field_type == "files")
                {
                    vals.set(field_name, entity.value(field_name, json::array()));
                }
            }
        }

        return std::nullopt;
    }

    std::string TableUnit::generateTableId(const std::string& tablename)
    {
        return "mt_" + std::to_string(std::hash<std::string>{}(tablename));
    }

    std::string TableUnit::getColTypeFromName(const std::string& col, const std::vector<json>& fields) const
    {
        for (const auto& field : fields)
        {
            if (field.value("name", "") == col && !col.empty())
                return field.value("type", "");
        }

        return "";
    }

    bool TableUnit::recordExists(const std::string& id) const
    {
        try
        {
            int count;
            const auto sql = MantisApp::instance().db().session();
            *sql << "SELECT COUNT(*) FROM " + m_tableName + " WHERE id = :id LIMIT 1",
                soci::use(id), soci::into(count);
            return count > 0;
        }
        catch (soci::soci_error& e)
        {
            Log::trace("TablesUnit::RecordExists error: {}", e.what());

            // If an error, return false. This means, if the id existed, we will end up throwing
            // UNIQUE violation error from the SQL side to avoid infinite looping
            return false;
        }
    }
}
