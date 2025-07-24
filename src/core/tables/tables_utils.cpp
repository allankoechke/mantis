//
// Created by allan on 07/06/2025.
//

#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../../include/mantis/utils/utils.h"

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
        const auto sql =  MantisApp::instance().db().session();

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
            const auto colType = getColTypeFromName(colName, m_fields);

            Log::trace("Parsing: #{} {} of type: {}", i, colName, colType);

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
                if (row.get_properties(i).get_db_type() == soci::db_date)
                {
                    auto t = row.get<std::tm>(i);
                    auto ts = DatabaseUnit::tmToISODate(t);
                    j[colName] = ts;
                }
                else
                {
                    j[colName] = row.get<std::string>(i, "");
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
                // TODO handle lists
                // j[colName] = row.get<std::string>(i);
            }
            else
            {
                // Throw an error for unknown types
                throw std::runtime_error(std::format("Unknown column type `{}` for column `{}`", colType, colName));
            }
        }

        return j;
    }

    std::string TableUnit::generateTableId(const std::string& tablename)
    {
        return "mt_" + std::to_string(std::hash<std::string>{}(tablename));
    }

    std::string TableUnit::getColTypeFromName(const std::string& col, const std::vector<json>& fields) const
    {
        for (const auto& field : fields)
        {
            // Log::trace("Field: {}, Type: {}, col: {}", field.at("name").get<std::string>(), field.at("type").get<std::string>(), col);
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
            const auto sql =  MantisApp::instance().db().session();
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
