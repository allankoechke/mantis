//
// Created by codeart on 09/11/2025.
//

#include "../../../include/mantis/core/models/entity.h"
#include "../../../include/mantis/utils/uuidv7.h"
#include "mantis/utils/soci_wrappers.h"

namespace mantis {
    Entity::Entity(const json &schema) {
        if (!schema.contains("name") || !schema.contains("type"))
            throw std::invalid_argument("Missing required fields `name` and `type` in schema!");

        m_schema = schema;

        // Ensure we have defaults for any missing fields so that T& does not fail
        if (!m_schema.contains("system")) m_schema["system"] = false;
        if (!m_schema.contains("has_api")) m_schema["has_api"] = true;
        if (!m_schema.contains("fields")) m_schema["fields"] = json::array();
        if (!m_schema.contains("list_rule")) m_schema["list_rule"] = "";
        if (!m_schema.contains("get_rule")) m_schema["get_rule"] = "";
        if (!m_schema.contains("add_rule")) m_schema["add_rule"] = "";
        if (!m_schema.contains("update_rule")) m_schema["update_rule"] = "";
        if (!m_schema.contains("delete_rule")) m_schema["delete_rule"] = "";
    }

    std::string Entity::id() const {
        return "mt_" + std::to_string(std::hash<std::string>{}(m_schema.at("id").get<std::string>()));
    }

    const std::string& Entity::name() const {
        return m_schema.at("name").get_ref<const std::string&>();
    }

    const std::string& Entity::type() const {
        return m_schema.at("type").get_ref<const std::string&>();
    }

    bool Entity::isSystem() const {
        return m_schema.contains("system") ? m_schema.at("system").get<bool>() : false;
    }

    bool Entity::hasApi() const {
        return m_schema.contains("has_api") ? m_schema.at("has_api").get<bool>() : false;
    }

    const std::vector<json>& Entity::fields() const {
        return m_schema.at("fields").get_ref<const std::vector<json>&>();
    }

    const std::string& Entity::listRule() const {
        return m_schema.at("list_rule").get_ref<const std::string&>();
    }

    const std::string& Entity::getRule() const {
        return m_schema.at("get_rule").get_ref<const std::string&>();
    }

    const std::string& Entity::addRule() const {
        return m_schema.at("add_rule").get_ref<const std::string&>();
    }

    const std::string& Entity::updateRule() const {
        return m_schema.at("update_rule").get_ref<const std::string&>();
    }

    const std::string& Entity::deleteRule() const {
        return m_schema.at("delete_rule").get_ref<const std::string&>();
    }

    // --------------------------------------------------------------------------- //
    // CRUD OPS                                                                    //
    // --------------------------------------------------------------------------- //

    Record Entity::create(const json &record, const json &opts) const {
        // Database session & transaction instance
        auto sql = MantisBase::instance().db().session();
        soci::transaction tr(*sql);

        try {
            // Create a random ID, then check if it exists already in the DB
            std::string id = generate_uuidv7();
            int trials = 1;
            while (recordExists(id)) {
                // Try getting a new ID ten times before giving up. Avoid infinite loops
                if (trials >= 10)
                    break;

                id = generate_uuidv7();
                trials++;
            }

            // Create default time values
            std::time_t current_t = time(nullptr);
            std::tm created_tm = *std::localtime(&current_t);
            std::string columns, placeholders;

            // Clone record for editing ...
            json new_record;

            // Force default values here ...
            new_record["id"] = "";
            new_record["created"] = nullptr;
            new_record["updated"] = nullptr;

            // Create the field cols and value cols as concatenated strings
            for (const auto &[field_name, _]: new_record.items()) {
                // First, ensure the key exists in our schema fields
                if (auto field_schema = findField(field_name); !field_schema.has_value()) continue;

                new_record[field_name] = record[field_name];
                columns += columns.empty() ? field_name : ", " + field_name;
                placeholders += placeholders.empty() ? (":" + field_name) : (", :" + field_name);
            }

            // Create the SQL Query
            std::string sql_query = "INSERT INTO " + name() + "(" + columns + ") VALUES (" + placeholders + ")";

            // Bind soci::values to entity values
            auto vals = json2SociValue(new_record, fields());
            vals.set("id", id, soci::i_ok);
            vals.set("created", created_tm, soci::i_ok);
            vals.set("updated", created_tm, soci::i_ok);

            // Execute sql query
            *sql << sql_query, soci::use(vals);
            tr.commit();

            // Query back the created record and send it back to the client
            soci::row r;
            *sql << "SELECT * FROM " + name() + " WHERE id = :id", soci::use(id), soci::into(r);
            auto added_row = sociRow2Json(r, fields());

            // Remove user password from the response
            if (type() == "auth") added_row.erase("password");
            return added_row;
        }
        catch (...) {
            // Handles anything else
            // logger::critical("Error executing Entity::create()");
            throw; // rethrow unknown exception
        }
    }

    Records Entity::list(const json &opts) const {
        const auto sql = MantisBase::instance().db().session();
        auto pagination = opts.value("pagination", json::object());
        // TODO ...
        // if (pagination.at("count_pages").get<bool>()) {
        //     int count = -1;
        //     // Let's count total records, unless switched off
        //     *sql << "SELECT COUNT(id) FROM " + name(), soci::into(count);
        // }

        // Extract the page number and page size
        const auto page = pagination.at("page_index").get<int>();
        const auto per_page = pagination.at("per_page").get<int>();

        if (per_page <= 0) throw std::invalid_argument("Page size, `per_page` value must be greater than 0");
        if (page <= 0) throw std::invalid_argument("Page number, `page` value must be greater than 0");

        const auto offset = (page - 1) * per_page;

        const auto query = "SELECT * FROM " + name() + " ORDER BY created DESC LIMIT :limit OFFSET :offset";
        const soci::rowset<soci::row> rs = (sql->prepare << query, soci::use(per_page), soci::use(offset));
        nlohmann::json record_list = nlohmann::json::array();

        for (const auto &row: rs) {
            auto row_json = sociRow2Json(row, fields());
            if (type() == "auth") {
                // Remove password fields from the response data
                row_json.erase("password");
            }
            record_list.push_back(row_json);
        }

        return record_list;
    }

    std::optional<Record> Entity::read(const std::string &id, const json &opts) const {
        // Get a soci::session from the pool
        const auto sql = MantisBase::instance().db().session();

        soci::row r; // To hold read data
        *sql << "SELECT * FROM " + name() + " WHERE id = :id", soci::use(id), soci::into(r);

        // If no data was found, return a std::nullopt
        if (!sql->got_data()) return std::nullopt; // 404

        // Parse returned record to JSON
        auto record = sociRow2Json(r, fields());

        // Remove user password from the response
        if (type() == "auth") record.erase("password");

        // Return the record
        return record;
    }

    Record Entity::update(const std::string &id, const json &data, const json &opts) const {
        // Database session & transaction instance
        auto sql = MantisBase::instance().db().session();
        soci::transaction tr(*sql);

        try {
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
            std::vector<std::string> updateFields;
            updateFields.reserve(data.size());

            // Create the field cols and value cols as concatenated strings
            for (const auto &[key, val]: data.items()) {
                // For system fields, let's ignore them for now.
                if (key == "id" || key == "created" || key == "updated") continue;

                // First, ensure the key exists in our schema fields
                auto schema = findField(key);
                if (!schema.has_value()) continue;

                columns += columns.empty() ? (key + " = :" + key) : (", " + key + " = :" + key);
                updateFields.push_back(key);

                // Track file fields for use later on
                if (schema.value()["type"] == "file" || schema.value()["type"] == "files") {
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

            // TODO, what do I return here?
            // Check that we have fields to update, if not so, just return
            // if (updateFields.empty()) {
            //     return result;
            // }

            // Add Updated field as an extra field for updates ...
            columns += columns.empty() ? ("updated = :updated") : (", updated = :updated");
            updateFields.emplace_back("updated");

            // Check for file(s) being saved from the request, determine if there is
            // need to delete/overwrite existing files
            if (!file_fields.empty()) {
                std::string fields_to_query{};

                for (const auto &file: file_fields) {
                    if (fields_to_query.empty()) fields_to_query = file["name"];
                    else fields_to_query += ", " + file["name"].get<std::string>();
                }

                const std::string sql_str = std::format("SELECT {} FROM {} WHERE id = :id LIMIT 1",
                                                        fields_to_query, name());

                soci::row r;
                *sql << sql_str, soci::use(id), soci::into(r);

                if (!sql->got_data()) {
                    throw std::runtime_error(std::format("Could not find record with id = {}", id));
                }

                // Parse soci::row to JSON object
                auto record = sociRow2Json(r, fields());

                // From the record, check for changes in files
                // Assuming record order is maintained on query ...
                for (const auto &file_field: file_fields) {
                    const auto field_name = file_field["name"].get<std::string>();

                    // For null values in db, continue
                    if (record[field_name].is_null()) continue;

                    const auto files_in_db = file_field["type"] == "files"
                                                 ? record[field_name]
                                                 : json::array({record[field_name]});

                    if (file_field["value"] == nullptr ||
                        (file_field["value"].is_array() && file_field["value"].size() == 0) ||
                        (file_field["value"].is_string() && file_field["value"].empty())) {
                        // If value set is null, add all file(s) to delete array
                        files_to_delete.insert(files_to_delete.end(), files_in_db.begin(), files_in_db.end());
                        continue;
                    }

                    const auto new_files = file_field["type"] == "files"
                                               ? file_field["value"]
                                               : json::array({file_field["value"]});

                    for (const auto &file: files_in_db) {
                        if (std::ranges::find(new_files, file) == new_files.end()) {
                            // The new list/file is missing the file named in the db, so delete it
                            files_to_delete.push_back(file);
                        }
                    }
                }
            }

            // Create the SQL Query
            std::string sql_query = "UPDATE " + name() + " SET " + columns + " WHERE id = :id";

            // Bind soci::values to entity values, throws an error if it fails
            auto vals = json2SociValue(data, fields());
            vals.set("id", id);
            vals.set("updated", created_tm);

            // Bind values, then execute
            *sql << sql_query, soci::use(vals);
            tr.commit();

            // Delete files, if any were removed ...
            for (const auto &file: files_to_delete) {
                if (!MantisBase::instance().files().removeFile(name(), file)) {
                    logger::warn("Could not delete file `{}` maybe it's missing?", file);
                }
            }

            // Query back the created record and send it back to the client
            soci::row r;
            *sql << "SELECT * FROM " + name() + " WHERE id = :id", soci::use(id), soci::into(r);
            Record new_record = sociRow2Json(r, fields());

            // Redact passwords
            if (type() == "auth") new_record.erase("password");
            return new_record;
        } catch (...) {
            throw;
        }
    }

    void Entity::remove(const std::string &id) const {
        // TRACE_CLASS_METHOD()
        // Views should not reach here
        if (type() == "view")
            return throw
                    std::invalid_argument("Remove is not implemented for Entity of `view` type!");

        const auto sql = MantisBase::instance().db().session();
        soci::transaction tr(*sql);

        // Check if item exists of given id
        soci::row row;
        const std::string sqlStr = ("SELECT * FROM " + name() + " WHERE id = :id LIMIT 1");
        *sql << sqlStr, soci::use(id), soci::into(row);

        if (!sql->got_data()) {
            throw std::runtime_error(std::format("Could not find record with id = {}", id));
        }

        // Remove from DB
        *sql << "DELETE FROM " + name() + " WHERE id = :id", soci::use(id);
        tr.commit();

        // Parse row to JSON
        const auto record = sociRow2Json(row, fields());

        // Extract all fields that have file/files as the underlying data
        std::vector<json> files_in_fields;
        std::ranges::for_each(fields(), [&](const json &field) {
            const auto &type = field["type"].get<std::string>();
            const auto &name = field["name"].get<std::string>();
            if (type == "file" && !record[name].is_null()) {
                const auto &file = record.value(name, "");
                if (!file.empty()) files_in_fields.emplace_back(file);
            }
            if (type == "files" && !record[name].is_null() && record[name].is_array()) {
                logger::trace("DEL FILES: '{}'", record[name].dump());

                const auto &files = record.value(name, std::vector<std::string>{});
                // Expand the array data out
                for (const auto &file: files) {
                    if (!file.empty()) files_in_fields.emplace_back(file);
                }
            }
        });

        // For each file field, remove it in the filesystem
        for (const auto &file_name: files_in_fields) {
            [[maybe_unused]]
                    auto _ = MantisBase::instance().files().removeFile(name(), file_name);
        }
    }

    const json &Entity::schema() const { return m_schema; }

    bool Entity::recordExists(const std::string &id) const {
        int count = -1;
        try {
            const auto sql = MantisBase::instance().db().session();
            *sql << "SELECT COUNT(id) FROM " + name() + " WHERE id = :id LIMIT 1",
                    soci::use(id), soci::into(count);
        } catch (soci::soci_error &e) {
            logger::trace("TablesUnit::RecordExists error: {}", e.what());
        }

        return count > 0;
    }

    std::optional<json> Entity::findField(const std::string &field_name) const {
        if (field_name.empty()) return std::nullopt;

        for (auto field: fields()) {
            if (field.value("name", "") == field_name) return field;
        }

        return std::nullopt;
    }

    std::optional<json> Entity::queryFromCols(const std::string &value, const std::vector<std::string> &columns) const {
        // Get a session object
        const auto sql = MantisBase::instance().db().session();

        // Build dynamic WHERE clause
        std::string where_clause;
        for (size_t i = 0; i < columns.size(); ++i) {
            if (i > 0) where_clause += " OR ";
            where_clause += columns[i] + " = :value";
        }

        const std::string query = "SELECT * FROM " + name() + " WHERE " + where_clause + " LIMIT 1";

        // Run query
        soci::row r;
        *sql << query, soci::use(value), soci::into(r);

        return sql->got_data() ? sociRow2Json(r, fields()) : std::nullopt;
    }
} // mantis
