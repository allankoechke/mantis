//
// Created by allan on 07/06/2025.
//

#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../include/mantis/utils.h"

namespace mantis
{
    json TableUnit::create(const json& entity, const json& opts)
    {
        Log::trace("TableMgr::create");
        json result;
        result["data"] = json::object();
        result["status"] = 201;
        result["error"] = "";

        // Database session & transaction instance
        auto sql = m_app->db().session();
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
                if (trials > 5)
                {
                    id = generateShortId(12 + trials % 5);
                }

                else
                {
                    id = generateShortId();
                }

                // Try getting a new ID ten times before giving up. Avoid infinite loops
                if (trials >= 10)
                    break;
            }

            // Create default time values
            std::time_t current_t = time(nullptr);
            std::tm* created_tm = std::localtime(&current_t);
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
            Log::trace("SQL Query: {}", sql_query);

            // Prepare statement
            soci::statement st = sql->prepare << sql_query;

            // Store all bound values to ensure lifetime
            std::vector<std::shared_ptr<void>> bound_values;
            soci::values vals;

            // Bind parameters dynamically
            for (const auto& field : m_fields)
            {
                const auto field_name = field.at("name").get<std::string>();

                if (field_name == "id")
                {
                    auto value = std::make_shared<std::string>(id.c_str());
                    bound_values.push_back(value);
                    soci::indicator ind = soci::i_ok;;
                    vals.set(field_name, *value, ind);
                }

                else if (field_name == "created" || field_name == "updated")
                {
                    auto value = std::make_shared<std::tm>(*created_tm);
                    bound_values.push_back(value);
                    soci::indicator ind = soci::i_ok;;
                    vals.set(field_name, *value, ind);
                }

                // For password types, let's hash them before binding to DB
                else if (field_name == "password")
                {
                    // TODO MOVE THIS SEGMENT TO A STRONG CRYPTOGRAPHIC ENGINE
                    // Maybe Argon | PBKDF2 | etc.

                    // Create SALT, append to the
                    const auto salt = generateShortId(); // Create SALT key
                    std::string pswd = entity.value(field_name, "");
                    const auto n_pswd = std::to_string(std::hash<std::string>{}(pswd + salt));

                    // Add the hashed password to the soci::vals
                    auto value = std::make_shared<std::string>(n_pswd + ":" + salt);
                    bound_values.push_back(value);
                    soci::indicator ind = soci::i_ok;;
                    vals.set(field_name, *value, ind);
                }

                else
                {
                    if (const auto field_type = field.at("type").get<std::string>();
                        field_type == "xml" || field_type == "string")
                    {
                        auto value = std::make_shared<std::string>(entity.value(field_name, ""));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "double")
                    {
                        auto value = std::make_shared<double>(entity.value(field_name, 0.0));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "date")
                    {
                        // TODO may throw an error?
                        std::tm tm{};
                        std::istringstream ss(entity.value(field_name, ""));
                        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

                        auto value = std::make_shared<std::tm>(tm);
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "int8")
                    {
                        auto value = std::make_shared<int8_t>(static_cast<int8_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "uint8")
                    {
                        auto value = std::make_shared<uint8_t>(static_cast<uint8_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "int16")
                    {
                        auto value = std::make_shared<int16_t>(static_cast<int16_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "uint16")
                    {
                        auto value = std::make_shared<uint16_t>(static_cast<uint16_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "int32")
                    {
                        auto value = std::make_shared<int32_t>(static_cast<int32_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "uint32")
                    {
                        auto value = std::make_shared<uint32_t>(static_cast<uint32_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "int64")
                    {
                        auto value = std::make_shared<int64_t>(static_cast<int64_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "uint64")
                    {
                        auto value = std::make_shared<uint64_t>(static_cast<uint64_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "blob")
                    {
                        auto value = std::make_shared<std::string>(entity.value(field_name, sql->empty_blob()));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "json")
                    {
                        auto value = std::make_shared<json>(entity.value(field_name, json::object()));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "bool")
                    {
                        auto value = std::make_shared<bool>(entity.value(field_name, false));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }
                }
            }

            st.bind(vals);
            st.execute(true);
            tr.commit();

            // Query back the created record and send it back to the client
            soci::row r;
            *sql << "SELECT * FROM " + m_tableName + " WHERE id = :id", soci::use(id), soci::into(r);
            const auto addedRow = parseDbRowToJson(r);

            result["error"] = "";
            result["data"] = addedRow;
            result["status"] = 201;

            return result;
        }
        catch (const soci::soci_error& e)
        {
            tr.rollback();

            result["error"] = e.what();
            result["status"] = 500;

            return result;
        } catch (const std::exception& e)
        {
            tr.rollback();

            json err;
            result["error"] = e.what();
            result["status"] = 500;

            return result;
        } catch (...)
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
        const auto sql = m_app->db().session();
        soci::row r;
        *sql << "SELECT * FROM " + m_tableName + " WHERE id = :id", soci::use(id), soci::into(r);

        if (!sql->got_data())
        {
            return std::nullopt;
        }

        return parseDbRowToJson(r);
    }

    json TableUnit::update(const std::string& id, const json& entity, const json& opts)
    {
        Log::trace("TableMgr::Update");

        json result;
        result["data"] = json::object();
        result["status"] = 200;
        result["error"] = "";

        // Database session & transaction instance
        auto sql = m_app->db().session();
        soci::transaction tr(*sql);

        try
        {
            // Create default time values
            std::time_t current_t = time(nullptr);
            std::tm* created_tm = std::localtime(&current_t);
            std::string columns, placeholders;

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
                if (!findFieldByKey(key).has_value()) continue;

                columns += columns.empty() ? (key + " = :" + key) : (", " + key + " = :" + key);
                updateFields.push_back(key);
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
            updateFields.push_back("updated");

            // Create the SQL Query
            std::string sql_query = "UPDATE " + m_tableName + " SET " + columns + " WHERE id = :id";
            Log::trace("SQL Query: {}", sql_query);

            // Prepare statement
            soci::statement st = sql->prepare << sql_query;

            // Store all bound values to ensure lifetime
            std::vector<std::shared_ptr<void>> bound_values;
            soci::values vals;

            // Bind parameters dynamically
            for (const auto& key : updateFields)
            {
                const auto field = findFieldByKey(key).value();
                const auto field_name = field.at("name").get<std::string>();

                // Just skip these fields
                if (key == "id" || key == "created") continue;

                if (key == "updated")
                {
                    auto value = std::make_shared<std::tm>(*created_tm);
                    bound_values.push_back(value);
                    soci::indicator ind = soci::i_ok;;
                    vals.set(field_name, *value, ind);
                }

                // For password types, let's hash them before binding to DB
                else if (field_name == "password")
                {
                    // TODO MOVE THIS SEGMENT TO A STRONG CRYPTOGRAPHIC ENGINE
                    // Maybe Argon | PBKDF2 | etc.

                    // Create SALT, append to the
                    const auto salt = generateShortId(); // Create SALT key
                    std::string pswd = entity.value(field_name, "");
                    const auto n_pswd = std::to_string(std::hash<std::string>{}(pswd + salt));

                    // Add the hashed password to the soci::vals
                    auto value = std::make_shared<std::string>(n_pswd + ":" + salt);
                    bound_values.push_back(value);
                    soci::indicator ind = soci::i_ok;;
                    vals.set(field_name, *value, ind);
                }

                else
                {
                    if (const auto field_type = field.at("type").get<std::string>();
                        field_type == "xml" || field_type == "string")
                    {
                        auto value = std::make_shared<std::string>(entity.value(field_name, ""));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "double")
                    {
                        auto value = std::make_shared<double>(entity.value(field_name, 0.0));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "date")
                    {
                        // TODO may throw an error?
                        std::tm tm{};
                        std::istringstream ss(entity.value(field_name, ""));
                        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

                        auto value = std::make_shared<std::tm>(tm);
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "int8")
                    {
                        auto value = std::make_shared<int8_t>(static_cast<int8_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "uint8")
                    {
                        auto value = std::make_shared<uint8_t>(static_cast<uint8_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "int16")
                    {
                        auto value = std::make_shared<int16_t>(static_cast<int16_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "uint16")
                    {
                        auto value = std::make_shared<uint16_t>(static_cast<uint16_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "int32")
                    {
                        auto value = std::make_shared<int32_t>(static_cast<int32_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "uint32")
                    {
                        auto value = std::make_shared<uint32_t>(static_cast<uint32_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "int64")
                    {
                        auto value = std::make_shared<int64_t>(static_cast<int64_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "uint64")
                    {
                        auto value = std::make_shared<uint64_t>(static_cast<uint64_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "blob")
                    {
                        auto value = std::make_shared<std::string>(entity.value(field_name, sql->empty_blob()));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "json")
                    {
                        auto value = std::make_shared<json>(entity.value(field_name, json::object()));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "bool")
                    {
                        auto value = std::make_shared<bool>(entity.value(field_name, false));
                        bound_values.push_back(value);
                        soci::indicator ind = soci::i_ok;;
                        vals.set(field_name, *value, ind);
                    }
                }
            }

            // Add binding for 'id'
            vals.set("id", id);

            // Bind values, then execute
            st.bind(vals);
            st.execute(true);
            tr.commit();

            // Query back the created record and send it back to the client
            soci::row r;
            *sql << "SELECT * FROM " + m_tableName + " WHERE id = :id", soci::use(id), soci::into(r);
            const auto record = parseDbRowToJson(r);

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
        } catch (const std::exception& e)
        {
            result["error"] = e.what();
            result["status"] = 500;

            return result;
        } catch (...)
        {
            result["error"] = "Unknown Error!";
            result["status"] = 500;

            return result;
        }

        return result;
    }

    bool TableUnit::remove(const std::string& id, const json& opts)
    {
        // TODO ensure views dont delete items
        const auto sql = m_app->db().session();
        soci::transaction tr(*sql);

        // Check if item exists of given id
        int count;
        const std::string sqlStr = ("SELECT count(*) FROM " + m_tableName + " WHERE id = :id LIMIT 1");
        *sql << sqlStr, soci::use(id), soci::into(count);

        if (count == 0)
        {
            throw std::runtime_error("Item with id = '" + id + "' was not found!");
        }

        // Remove from DB
        *sql << "DELETE FROM " + m_tableName + " WHERE id = :id", soci::use(id);
        Log::trace("SQL Query: {}", sql->get_query());

        tr.commit();

        // TODO reload routes

        return true;
    }

    std::vector<json> TableUnit::list(const json& opts)
    {
        const auto sql = m_app->db().session();
        const soci::rowset<soci::row> rs = (sql->prepare << "SELECT * FROM " + tableName());
        nlohmann::json response = nlohmann::json::array();

        for (const auto& row : rs)
        {
            auto rowJson = parseDbRowToJson(row);
            response.push_back(rowJson);
        }

        return response;
    }
}
