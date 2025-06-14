//
// Created by allan on 07/06/2025.
//

#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../include/mantis/utils.h"

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
        const auto sql = m_app->db().session();

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
        json j;
        for (size_t i = 0; i < row.size(); i++)
        {
            const auto colName = row.get_properties(i).get_name();

            if (const std::string colType = getColTypeFromName(colName);
                colType == "xml" || colType == "string")
            {
                j[colName] = row.get<std::string>(i, "");
            }
            else if (colType == "double")
            {
                j[colName] = row.get<double>(i);
            }
            else if (colType == "date")
            {
                if (row.get_properties(i).get_db_type() == soci::db_date)
                {
                    auto t = row.get<std::tm>(i);
                    auto ts = DatabaseUnit::tmToISODate(t);

                    std::cout << "Date String: " << ts << std::endl;
                    j[colName] = ts;
                }
                else // Parse as string regardless
                {
                    try
                    {
                        // Date stored as string or in integer column - get as string and parse manually
                        auto date_str = row.get<std::string>(i);

                        // Use SOCI's internal parser
                        // soci::details::parse_std_tm(date_str.c_str(), t);
                        j[colName] = date_str;
                    }
                    catch (soci::soci_error& e)
                    {
                        j[colName] = "";
                        Log::critical("TablesUnit::Parse DB Row Error: {}", e.what());
                        // throw std::runtime_error(e.what());
                    }
                    catch (std::exception& e)
                    {
                        j[colName] = "";
                        Log::critical("TablesUnit::Parse DB Row Error: {}", e.what());
                        // throw std::runtime_error(e.what());
                    }
                }
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
                j[colName] = row.get<std::string>(i);
            }
            else if (colType == "json")
            {
                j[colName] = row.get<json>(i);
            }
            else if (colType == "bool")
            {
                j[colName] = row.get<bool>(i);
            }
            else // Return a string for unknown types // TODO avoid errors
            {
                j[colName] = row.get<std::string>(i);
            }
        }

        return j;
    }

    TableValue TableUnit::getTypedValue(const json& row, const std::string& colName, const std::string& type)
    {
        if (!row.contains(colName) || row[colName].is_null())
            return std::monostate{};

        const json& value = row[colName];

        try
        {
            if (type == "double")
                return value.get<double>();
            if (type == "bool")
                return value.get<bool>();
            if (type == "int8")
                return static_cast<int8_t>(value.get<int>());
            if (type == "uint8")
                return static_cast<uint8_t>(value.get<int>());
            if (type == "int16")
                return static_cast<int16_t>(value.get<int>());
            if (type == "uint16")
                return static_cast<uint16_t>(value.get<int>());
            if (type == "int32")
                return value.get<int32_t>();
            if (type == "uint32")
                return value.get<uint32_t>();
            if (type == "int64")
                return value.get<int64_t>();
            if (type == "uint64")
                return value.get<uint64_t>();
            if (type == "json")
                return value;
            if (type == "date")
            {
                std::tm tm{};
                std::istringstream ss(value.get<std::string>());
                ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
                return tm;
            }

            // TODO Add BLOB and XML support here as well, this as is may throw an error

            // Unknown type, fallback to string
            return value.get<std::string>();
        }
        catch (const std::exception&)
        {
            return std::monostate{}; // or throw if preferred
        }
    }

    std::string TableUnit::generateTableId(const std::string& tablename)
    {
        return "mt_" + std::to_string(std::hash<std::string>{}(tablename));
    }

    std::string TableUnit::getColTypeFromName(const std::string& col) const
    {
        for (const auto& field : m_fields)
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
            const auto sql = m_app->db().session();
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
