//
// Created by allan on 07/06/2025.
//

#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../../include/mantis/utils/utils.h"

#define __file__ "core/tables/tables_crud.cpp"

#include <cmath>

#include "mantis/core/fileunit.h"

namespace mantis
{
    json TableUnit::create(const json& entity, const json& opts)
    {
        TRACE_CLASS_METHOD()

        json result;
        result["data"] = json::object();
        result["status"] = 201;
        result["error"] = "";

        // Database session & transaction instance
        auto sql = MantisApp::instance().db().session();
        soci::transaction tr(*sql);

        try
        {
            // Create a random ID, then check if it exists already in the DB
            std::string id = generateShortId();
            int trials = 0;
            while (recordExists(id))
            {
                trials++;

                //  As trials go over 5, expand the id size by a character every time
                if (trials > 5) id = generateShortId(12 + trials % 5);
                else id = generateShortId();

                // Try getting a new ID ten times before giving up. Avoid infinite loops
                if (trials >= 10)
                    break;
            }

            // Create default time values
            std::time_t current_t = time(nullptr);
            std::tm created_tm = *std::localtime(&current_t);
            std::string columns, placeholders;

            // Create the field cols and value cols as concatenated strings
            for (const auto& field : m_fields)
            {
                const auto field_name = field.at("name").get<std::string>();
                columns += columns.empty() ? field_name : ", " + field_name;
                placeholders += placeholders.empty() ? (":" + field_name) : (", :" + field_name);
            }

            // Create the SQL Query
            std::string sql_query = "INSERT INTO " + m_tableName + "(" + columns + ") VALUES (" + placeholders + ")";

            // Store all bound values to ensure lifetime
            soci::values vals;

            soci::indicator ind = soci::i_ok;
            vals.set("id", id, ind);
            vals.set("created", created_tm, ind);
            vals.set("updated", created_tm, ind);

            // Bind soci::values to entity values
            const auto status = bindEntityToSociValue(vals, entity);
            if (status.has_value())
            {
                return status.value();
            }

            // Execute sql query
            *sql << sql_query, soci::use(vals);
            tr.commit();

            // Query back the created record and send it back to the client
            soci::row r;
            *sql << "SELECT * FROM " + m_tableName + " WHERE id = :id", soci::use(id), soci::into(r);
            auto added_row = parseDbRowToJson(r);

            Log::trace("Added record: {}", added_row.dump());

            // Remove user password from the response
            if (tableType() == "auth") added_row.erase("password");

            result["error"] = "";
            result["data"] = added_row;
            result["status"] = 201;

            return result;
        }
        catch (const soci::soci_error& e)
        {
            tr.rollback();

            result["error"] = e.what();
            result["status"] = 500;

            return result;
        }
        catch (const std::exception& e)
        {
            tr.rollback();

            json err;
            result["error"] = e.what();
            result["status"] = 500;

            return result;
        }
        catch (...)
        {
            tr.rollback();

            json err;
            result["error"] = "Unknown Error!";
            result["status"] = 500;

            return result;
        }

        return result;
    }

    std::optional<json> TableUnit::read(const std::string& id, const json& opts)
    {
        TRACE_CLASS_METHOD()

        // Get a soci::session from the pool
        const auto sql = MantisApp::instance().db().session();

        soci::row r; // To hold read data
        *sql << "SELECT * FROM " + m_tableName + " WHERE id = :id", soci::use(id), soci::into(r);

        // If no data was found, return a nullopt
        if (!sql->got_data()) return std::nullopt;

        // Parse returned record to JSON
        auto record = parseDbRowToJson(r);

        // Remove user password from the response
        if (tableType() == "auth") record.erase("password");

        // Return the record
        return record;
    }

    json TableUnit::update(const std::string& id, const json& entity, const json& opts)
    {
        TRACE_CLASS_METHOD()

        json result;
        result["data"] = json::object();
        result["status"] = 200;
        result["error"] = "";

        // Database session & transaction instance
        auto sql = MantisApp::instance().db().session();
        soci::transaction tr(*sql);

        try
        {
            // Create default time values
            std::time_t current_t = time(nullptr);
            std::tm created_tm = *std::localtime(&current_t);
            std::string columns, placeholders;

            // Store files to delete by filename
            std::vector<std::string> files_to_delete{};
            std::vector<json> file_fields{};

            // Create a temporary container to track fields we intend to update.
            // Why? We'll limit to the fields we have in our schema, that way, we
            // don't have any surprises.
            // TODO Maybe open it up? But how do we handle other types?
            std::vector<std::string> updateFields;
            updateFields.reserve(entity.size());

            // Create the field cols and value cols as concatenated strings
            for (const auto& [key, val] : entity.items())
            {
                // For system fields, let's ignore them for now.
                if (key == "id" || key == "created" || key == "updated") continue;

                // First, ensure the key exists in our schema fields
                auto schema = findFieldByKey(key);
                if (!schema.has_value()) continue;

                columns += columns.empty() ? (key + " = :" + key) : (", " + key + " = :" + key);
                updateFields.push_back(key);

                // Track file fields for use later on
                if (schema.value()["type"] == "file" || schema.value()["type"] == "files")
                {
                    file_fields.push_back(
                        json{
                            {"name", key},
                            {"value", val},
                            {
                                "type", schema.value()["type"]
                            }
                        });
                }
            }

            // Check that we have fields to update, if not so, just return
            if (updateFields.empty())
            {
                result["error"] = "Nothing to update";
                result["data"] = json::object();
                result["status"] = 200;
                return result;
            }

            // Add Updated field as an extra field for updates ...
            columns += columns.empty() ? ("updated = :updated") : (", updated = :updated");
            updateFields.emplace_back("updated");

            // Check for file(s) being saved from the request, determine if there is
            // need to delete/overwrite existing files
            if (!file_fields.empty())
            {
                std::string fields_to_query{};

                for (const auto& file : file_fields)
                {
                    if (fields_to_query.empty()) fields_to_query = file["name"];
                    else fields_to_query += ", " + file["name"].get<std::string>();
                }

                const std::string sql_str = std::format("SELECT {} FROM {} WHERE id = :id LIMIT 1",
                                                        fields_to_query, m_tableName);

                soci::row r;
                *sql << sql_str, soci::use(id), soci::into(r);

                if (!sql->got_data())
                {
                    result["error"] = std::format("Could not find record with id = {}", id);
                    result["status"] = 500;
                    return result;
                }

                // Parse soci::row to JSON object
                auto record = parseDbRowToJson(r);

                // From the record, check for changes in files
                // Assuming record order is maintained on query ...
                for (const auto& file_field : file_fields)
                {
                    const auto field_name = file_field["name"].get<std::string>();

                    // For null values in db, continue
                    if (record[field_name].is_null()) continue;

                    const auto files_in_db = file_field["type"] == "files"
                                                 ? record[field_name]
                                                 : json::array({record[field_name]});

                    if (file_field["value"] == nullptr ||
                        (file_field["value"].is_array() && file_field["value"].size() == 0) ||
                        (file_field["value"].is_string() && file_field["value"].empty()))
                    {
                        // If value set is null, add all file(s) to delete array
                        files_to_delete.insert(files_to_delete.end(), files_in_db.begin(), files_in_db.end());
                        continue;
                    }

                    const auto new_files = file_field["type"] == "files"
                                               ? file_field["value"]
                                               : json::array({file_field["value"]});

                    for (const auto& file : files_in_db)
                    {
                        if (std::ranges::find(new_files, file) == new_files.end())
                        {
                            // The new list/file is missing the file named in the db, so delete it
                            files_to_delete.push_back(file);
                        }
                    }
                }
            }

            // Create the SQL Query
            std::string sql_query = "UPDATE " + m_tableName + " SET " + columns + " WHERE id = :id";

            // Store values for binding
            soci::values vals;

            vals.set("id", id);
            vals.set("updated", created_tm);

            // Bind soci::values to entity values
            // Check if the return has a value, if yes, return the value
            if (const auto status = bindEntityToSociValue(vals, entity);
                status.has_value())
            {
                // An error occurred while binding data to soci::values
                return status.value();
            }

            // Bind values, then execute
            *sql << sql_query, soci::use(vals);
            // Log::trace(">> $ sql << {}\n\t└── Values ({})", sql->get_query(), sql->get_last_query_context());
            tr.commit();

            // Delete files, if any were removed ...
            for (const auto& file : files_to_delete)
            {
                if (!MantisApp::instance().files().removeFile(m_tableName, file))
                {
                    Log::warn("Could not delete file, is it missing?\n\t- `{}`", file);
                }
            }

            // Query back the created record and send it back to the client
            soci::row r;
            *sql << "SELECT * FROM " + m_tableName + " WHERE id = :id", soci::use(id), soci::into(r);
            auto record = parseDbRowToJson(r);

            // Redact passwords
            if (tableType() == "auth") record.erase("password");

            result["error"] = "";
            result["data"] = record;
            result["status"] = 200;

            return result;
        }
        catch (const soci::soci_error& e)
        {
            result["error"] = e.what();
            result["status"] = 500;

            return result;
        }
        catch (const std::exception& e)
        {
            result["error"] = e.what();
            result["status"] = 500;

            return result;
        }
        catch (...)
        {
            result["error"] = "Unknown Error!";
            result["status"] = 500;

            return result;
        }

        return result;
    }

    bool TableUnit::remove(const std::string& id, const json& opts)
    {
        TRACE_CLASS_METHOD()
        // Views should not reach here
        if (tableType() == "view") return false;

        const auto sql = MantisApp::instance().db().session();
        soci::transaction tr(*sql);

        // Check if item exists of given id
        soci::row row;
        const std::string sqlStr = ("SELECT * FROM " + m_tableName + " WHERE id = :id LIMIT 1");
        *sql << sqlStr, soci::use(id), soci::into(row);

        if (!sql->got_data())
        {
            throw std::runtime_error(std::format("Could not find record with id = {}", id));
        }

        // Remove from DB
        *sql << "DELETE FROM " + m_tableName + " WHERE id = :id", soci::use(id);
        tr.commit();

        // Parse row to JSON
        const auto record = parseDbRowToJson(row);

        // Extract all fields that have file/files as the underlying data
        std::vector<json> files_in_fields;
        std::ranges::for_each(m_fields, [&](const json& field)
        {
            const auto& type = field["type"].get<std::string>();
            const auto& name = field["name"].get<std::string>();
            if (type == "file" && !record[name].is_null())
            {
                const auto& file = record.value(name, "");
                if (!file.empty()) files_in_fields.push_back(file);
            }
            if (type == "files" && !record[name].is_null() && record[name].is_array())
            {
                std::cout << "DEL FILES: " << record[name].dump() << std::endl;

                const auto& files = record.value(name, std::vector<std::string>{});
                // Expand the array data out
                for (const auto& file : files)
                {
                    if (!file.empty()) files_in_fields.push_back(file);
                }
            }
        });

        // For each file field, remove it in the filesystem
        for (const auto& file_name : files_in_fields)
        {
            [[maybe_unused]]
                auto _ = MantisApp::instance().files().removeFile(m_tableName, file_name);
        }
        return true;
    }

    json TableUnit::list_records(const json& opts)
    {
        TRACE_CLASS_METHOD()
        json response = {{"error", ""}, {"pagination", json::object()}, {"data", json::array()}};
        const auto sql = MantisApp::instance().db().session();

        auto pagination = opts.value("pagination", json::object());
        int count = -1;
        if (pagination.at("countPages").get<bool>())
        {
            // Let's count total records, unless switched off
            // TODO this assumes all tables have `id`, which should for now
            *sql << "SELECT COUNT(id) FROM " + tableName(), soci::into(count);
        }

        // Extract the page number and page size
        const auto page = pagination.at("pageIndex").get<int>();
        const auto perPage = pagination.at("perPage").get<int>();

        if (perPage <= 0)
        {
            response["error"] = "Page size must be greater than 0";
            return response;
        }
        if (page <= 0)
        {
            response["error"] = "Page index must be greater than 0";
            return response;
        }
        const auto offset = (page - 1) * perPage;

        const auto query = "SELECT * FROM " + tableName() + " ORDER BY created DESC LIMIT :limit OFFSET :offset";
        const soci::rowset<soci::row> rs = (sql->prepare << query, soci::use(perPage), soci::use(offset));
        nlohmann::json list = nlohmann::json::array();

        for (const auto& row : rs)
        {
            auto row_json = parseDbRowToJson(row);
            if (m_tableType == "auth")
            {
                // Remove password fields from the response data
                row_json.erase("password");
            }
            list.push_back(row_json);
        }


        // Update pagination data
        pagination.erase("countPages");
        pagination["pageCount"] = count == -1
                                      ? count
                                      : static_cast<int>(std::ceil(static_cast<double>(count) / perPage));
        pagination["recordCount"] = count;

        // Set response data
        response["data"] = list;
        response["pagination"] = pagination;

        return response;
    }
}
