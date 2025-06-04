//
// Created by allan on 13/05/2025.
//

#include "../../../include/mantis/core/models/tables.h"
#include "../../../include/mantis/core/database.h"
#include "../../../include/mantis/app/app.h"
#include "../../../include/mantis/utils.h"
#include "spdlog/fmt/bundled/compile.h"

namespace mantis
{
    TableUnit::TableUnit(
        MantisApp* app,
        std::string tableName,
        std::string tableId,
        std::string tableType)
        : m_app(app),
          m_tableName(std::move(tableName)),
          m_tableId(std::move(tableId)),
          m_tableType(std::move(tableType)),
          m_listRule(Rule{}),
          m_getRule(Rule{}), m_addRule(Rule{}),
          m_updateRule(Rule{}), m_deleteRule(Rule{})
    {
    }

    TableUnit::TableUnit(
        MantisApp* app,
        const json& schema)
        : m_app(app),
          m_listRule(Rule{}),
          m_getRule(Rule{}),
          m_addRule(Rule{}),
          m_updateRule(Rule{}),
          m_deleteRule(Rule{})
    {
        fromJson(schema);
    }

    void TableUnit::setRouteDisplayName(const std::string& routeName)
    {
        if (routeName.empty())
            return;

        m_routeName = routeName;
    }

    bool TableUnit::setupRoutes()
    {
        if (m_tableName.empty() && m_routeName.empty()) return false;

        const auto path = m_routeName.empty() ? m_tableName : m_routeName;
        const auto basePath = "/api/v1/" + path;

        try
        {
            // Fetch All Records
            Log::debug("Adding GET Request for table '{}'", m_tableName);
            m_app->http().Get(
                basePath,
                [this](const Request& req, Response& res, Context& ctx)-> void
                {
                    fetchRecords(req, res, ctx);
                },
                {
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return getAuthToken(req, res, ctx);
                    },
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return hasAccess(req, res, ctx);
                    }
                }
            );

            // Fetch Single Record
            Log::debug("Adding GET/1 Request for table '{}'", m_tableName);
            m_app->http().Get(
                basePath + "/:id",
                [this](const Request& req, Response& res, Context& ctx)-> void
                {
                    fetchRecord(req, res, ctx);
                },
                {
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return getAuthToken(req, res, ctx);
                    },
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return hasAccess(req, res, ctx);
                    }
                }
            );

            // Add/Update and Delete are not supported in views
            if (m_tableType != "view")
            {
                // Add Record
                Log::debug("Adding POST Request for table '{}'", m_tableName);
                m_app->http().Post(
                    basePath, [this](const Request& req, Response& res, Context& ctx)-> void
                    {
                        createRecord(req, res, ctx);
                    },
                    {
                        [this](const Request& req, Response& res, Context& ctx)-> bool
                        {
                            return getAuthToken(req, res, ctx);
                        },
                        [this](const Request& req, Response& res, Context& ctx)-> bool
                        {
                            return hasAccess(req, res, ctx);
                        }
                    }
                );

                // Update Record
                Log::debug("Adding PATCH Request for table '{}'", m_tableName);
                m_app->http().Patch(
                    basePath + "/:id",
                    [this](const Request& req, Response& res, Context& ctx)-> void
                    {
                        updateRecord(req, res, ctx);
                    },
                    {
                        [this](const Request& req, Response& res, Context& ctx)-> bool
                        {
                            return getAuthToken(req, res, ctx);
                        },
                        [this](const Request& req, Response& res, Context& ctx)-> bool
                        {
                            return hasAccess(req, res, ctx);
                        }
                    }
                );

                // Delete Record
                Log::debug("Adding DELETE Request for table '{}'", m_tableName);
                m_app->http().Delete(
                    basePath + "/:id",
                    [this](const Request& req, Response& res, Context& ctx)-> void
                    {
                        deleteRecord(req, res, ctx);
                    },
                    {
                        [this](const Request& req, Response& res, Context& ctx)-> bool
                        {
                            return getAuthToken(req, res, ctx);
                        },
                        [this](const Request& req, Response& res, Context& ctx)-> bool
                        {
                            return hasAccess(req, res, ctx);
                        }
                    }
                );
            }

            return true;
        }

        catch (const std::exception& e)
        {
            Log::critical("Failed to create routes for table '{}' of '{}' type: {}",
                          m_tableName, m_tableType, e.what());
            return false;
        }

        catch (...)
        {
            Log::critical("Failed to create routes for table '{}' of '{}' type: {}",
                          m_tableName, m_tableType, "Unknown Error!");
            return false;
        }
    }


    void TableUnit::fetchRecord(const Request& req, Response& res, Context& ctx)
    {
        Log::debug("TableMgr::FetchRecord for {}", req.path);
        const auto id = req.path_params.at("id");
        json response;
        if (id.empty())
        {
            response["status"] = 400;
            response["error"] = "Record ID is required";
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }

        try
        {
            if (const auto resp = read(id, json::object()); resp.has_value())
            {
                response["status"] = 200;
                response["error"] = "";
                response["data"] = resp.value();

                res.set_content(response.dump(), "application/json");
                res.status = 200;
                return;
            }

            response["status"] = 404;
            response["error"] = "Item Not Found";
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 404;
        }

        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["error"] = e.what();
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 500;
        }

        catch (...)
        {
            response["status"] = 500;
            response["error"] = "Unknown Error";
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 500;
        }
    }

    void TableUnit::fetchRecords(const Request& req, Response& res, Context& ctx)
    {
        Log::trace("TableMgr::FetchRecords for {}", req.path);
        json response;
        try
        {
            // TODO do pagination of the data
            auto resp = list(json::object());
            response["data"] = resp;
            response["status"] = 200;
            response["error"] = "";

            res.status = 200;
            res.set_content(response.dump(), "application/json");
        }

        catch (const std::exception& e)
        {
            response["data"] = json::array();
            response["status"] = 500;
            response["error"] = e.what();

            res.status = 500;
            res.set_content(response.dump(), "application/json");
        }

        catch (...)
        {
            response["data"] = json::array();
            response["status"] = 500;
            response["error"] = "Internal Server Error";

            res.status = 500;
            res.set_content(response.dump(), "application/json");
        }
    }

    void TableUnit::createRecord(const Request& req, Response& res, Context& ctx)
    {
        Log::debug("TableMgr::CreateRecord for {}", req.path);
        json body, response;
        try
        {
            body = json::parse(req.body);
        }
        catch (const std::exception& e)
        {
            response["status"] = 400;
            response["error"] = e.what();
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }

        Log::debug("Create record for table {} with data = {}", tableName(), body.dump());

        // Validate JSON body
        if ( const auto resp = validateRequestBody(body) )
        {
            response["status"] = resp.value().value("status", 500);
            response["error"] = resp.value().value("error", "");
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = resp.value().value("status", 500);
            Log::critical("Error Validating Field: {}", resp.value().value("error", ""));

            return;
        };

        auto respObj = create(body, json{});
        if (!respObj.value("error", "").empty())
        {
            int status = respObj.value("status", 500);
            response["status"] = status;
            response["error"] = respObj.at("error").get<std::string>();
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = status;

            Log::critical("Failed to create record, reason: {}", respObj.dump());
            return;
        }

        Log::trace("Record creation successful");
        response["status"] = 201;
        response["error"] = "";
        response["data"] = respObj.at("data");

        res.set_content(response.dump(), "application/json");
        res.status = 201;
    }

    void TableUnit::updateRecord(const Request& req, Response& res, Context& ctx)
    {
        Log::debug("TableMgr::UpdateRecord for {}", req.path);
        ctx.dump();

        json response;
        response["status"] = 200;
        response["body"] = json::object();
        response["message"] = "Record Updated";

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    }

    void TableUnit::deleteRecord(const Request& req, Response& res, Context& ctx)
    {
        Log::debug("TableMgr::DeleteRecord for {}", req.path);
        ctx.dump();

        json response;
        response["status"] = 204;
        response["body"] = json::object();
        response["message"] = "Record Deleted";

        res.status = 204;
        res.set_content(response.dump(), "application/json");
    }

    bool TableUnit::authWithPassword(const std::string& email, std::string& password)
    {
        return true;
    }

    bool TableUnit::getAuthToken(const Request& req, [[maybe_unused]] Response& res, Context& ctx)
    {
        Log::trace("TableMgr::HasAuthHeader for {}", req.path);

        // If we have an auth header, extract it into the ctx, else
        // add a guest user type. The auth if present, should have
        // the user id, auth table, etc.
        json auth;
        auth["token"] = "";
        auth["type"] = "guest"; // or 'user'
        auth["user"] = json::object();

        if (req.has_header("Authorization"))
        {
            auth["token"] = get_bearer_token_auth(req);
            auth["type"] = "user";

            // Maybe Query User information if it exists?
            // id, tableName, ...
        }

        // Update the context
        ctx.set("auth", auth);
        ctx.dump();

        return true;
    }

    bool TableUnit::hasAccess(const Request& req, Response& res, Context& ctx)
    {
        Log::debug("TableMgr::HasAccessPermission for {}", req.path);
        ctx.dump();
        return true;
    }

    std::string TableUnit::tableName()
    {
        return m_tableName;
    }

    void TableUnit::setTableName(const std::string& name)
    {
        m_tableName = name;
    }

    std::string TableUnit::tableId()
    {
        return m_tableId;
    }

    void TableUnit::setTableId(const std::string& id)
    {
        m_tableId = id;
    }

    std::string TableUnit::tableType()
    {
        return m_tableType;
    }

    void TableUnit::fromJson(const json& j)
    {
        m_tableName = "";
        m_tableId = "";

        m_fields.clear();
        m_fields = j.value("fields", json::array());

        m_listRule = j.value("listRule", "");
        m_getRule = j.value("getRule", "");
        m_addRule = j.value("addRule", "");
        m_updateRule = j.value("updateRule", "");
        m_deleteRule = j.value("deleteRule", "");

        m_isSystem = j.value("system", false);
        m_tableType = j.value("type", "base");
    }

    std::vector<json> TableUnit::fields() const
    {
        return m_fields;
    }

    void TableUnit::setFields(const std::vector<json>& fields)
    {
        m_fields.clear();
        for (const auto field : fields) m_fields.push_back(field);
    }

    bool TableUnit::isSystem() const
    {
        return m_isSystem;
    }

    void TableUnit::setIsSystemTable(const bool isSystemTable)
    {
        m_isSystem = isSystemTable;
    }

    Rule TableUnit::listRule()
    {
        return m_listRule;
    }

    void TableUnit::setListRule(const Rule& rule)
    {
        m_listRule = rule;
    }

    Rule TableUnit::getRule()
    {
        return m_getRule;
    }

    void TableUnit::setGetRule(const Rule& rule)
    {
        m_getRule = rule;
    }

    Rule TableUnit::addRule()
    {
        return m_addRule;
    }

    void TableUnit::setAddRule(const Rule& rule)
    {
        m_addRule = rule;
    }

    Rule TableUnit::updateRule()
    {
        return m_updateRule;
    }

    void TableUnit::setUpdateRule(const Rule& rule)
    {
        m_updateRule = rule;
    }

    Rule TableUnit::deleteRule()
    {
        return m_deleteRule;
    }

    void TableUnit::setDeleteRule(const Rule& rule)
    {
        m_deleteRule = rule;
    }

    std::string TableUnit::generateTableId(const std::string& tablename)
    {
        return "mt_" + std::to_string(std::hash<std::string>{}(tablename));
    }

    json TableUnit::create(const json& entity, const json& opts)
    {
        Log::trace("TableMgr::create");
        json result;
        result["data"] = json::object();
        result["status"] = 200;
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
                if (trials >  5)
                {
                    id = generateShortId( 12 + trials%5);
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
                placeholders += placeholders.empty() ? (":"+field_name) : (", :" + field_name);
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
                    soci::indicator ind;
                    vals.set(field_name, *value, ind);
                }

                else if (field_name == "created" || field_name == "updated")
                {
                    auto value = std::make_shared<std::tm>(*created_tm);
                    bound_values.push_back(value);
                    soci::indicator ind;
                    vals.set(field_name, *value, ind);
                }

                else
                {
                    if (const auto field_type = field.at("type").get<std::string>();
                        field_type == "xml" || field_type == "string")
                    {
                        auto value = std::make_shared<std::string>(entity.value(field_name, ""));
                        bound_values.push_back(value);
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "double")
                    {
                        auto value = std::make_shared<double>(entity.value(field_name, 0.0));
                        bound_values.push_back(value);
                        soci::indicator ind;
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
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "int8")
                    {
                        auto value = std::make_shared<int8_t>(static_cast<int8_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "uint8")
                    {
                        auto value = std::make_shared<uint8_t>(static_cast<uint8_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "int16")
                    {
                        auto value = std::make_shared<int16_t>(static_cast<int16_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "uint16")
                    {
                        auto value = std::make_shared<uint16_t>(static_cast<uint16_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "int32")
                    {
                        auto value = std::make_shared<int32_t>(static_cast<int32_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "uint32")
                    {
                        auto value = std::make_shared<uint32_t>(static_cast<uint32_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "int64")
                    {
                        auto value = std::make_shared<int64_t>(static_cast<int64_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "uint64")
                    {
                        auto value = std::make_shared<uint64_t>(static_cast<uint64_t>(entity.value(field_name, 0)));
                        bound_values.push_back(value);
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "blob")
                    {
                        auto value = std::make_shared<std::string>(entity.value(field_name, sql->empty_blob()));
                        bound_values.push_back(value);
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "json")
                    {
                        auto value = std::make_shared<json>(entity.value(field_name, json::object()));
                        bound_values.push_back(value);
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }

                    else if (field_type == "bool")
                    {
                        auto value = std::make_shared<bool>(entity.value(field_name, false));
                        bound_values.push_back(value);
                        soci::indicator ind;
                        vals.set(field_name, *value, ind);
                    }
                }
            }

            st.bind(vals);
            st.execute(true);
            tr.commit();

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
        *sql << "SELECT * FROM " + tableName() + " WHERE id = :id", soci::use(id), soci::into(r);

        if (!sql->got_data())
        {
            return nullopt;
        }

        return parseDbRowToJson(r);
    }

    json TableUnit::update(const std::string& id, const json& entity, const json& opts)
    {
        return entity;
    }

    bool TableUnit::remove(const std::string& id, const json& opts)
    {
        // TODO ensure views dont delete items
        const auto sql = m_app->db().session();
        soci::transaction tr(*sql);

        // Check if item exists of given id
        soci::row r;
        *sql << "SELECT * FROM __tables WHERE id = :id LIMIT 1", soci::use(id), soci::into(r);

        if (!sql->got_data())
        {
            throw std::runtime_error("Item with id = '" + id + "' was not found!");
        }

        // Remove from DB
        *sql << "DELETE FROM " + tableName() + " WHERE id = :id", soci::use(id);

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
        Log::trace("TablesUnit::RecordExists for {} {}", m_tableName, id);
        try
        {
            int count;
            const auto sql = m_app->db().session();
            const std::string query = "SELECT COUNT(*) FROM " + m_tableName + " WHERE id = :id LIMIT 1";
            *sql << query, soci::use(id), soci::into(count);
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

    json TableUnit::parseDbRowToJson(const soci::row& row) const
    {
        json j;
        for (int i = 0; i < row.size(); i++)
        {
            const auto colName = row.get_properties(i).get_name();
            const std::string colType = getColTypeFromName(colName);

            if (colType == "xml")
            {
                j[colName] = row.get<soci::xml_type>(i).value;
            }
            else if (colType == "string")
            {
                j[colName] = row.get<std::string>(i);
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
                    }catch (soci::soci_error& e)
                    {
                        j[colName] = "";
                        Log::critical("TablesUnit::parseDbRowToJson Date Parse Error: {}", e.what());
                        throw std::runtime_error("Failed to parse date");
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

    std::optional<json> TableUnit::validateRequestBody(const json& body) const
    {
        // Create default base object
        for (const auto& field : m_fields)
        {
            json obj;
            const auto& name = field.value("name", "");

            // Skip system generated fields
            if (name=="id" || name=="created" || name=="updated") continue;

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

                Log::trace("Min Value check ... 01");
                if (type == "string" && body.at(name).size() < minValue)
                {
                    obj["error"] = "String value should be at least " + std::to_string(minValue) + " characters long.";
                    obj["status"] = 400;
                    return obj;
                }

                Log::trace("Min Value check ... 02");
                if (type == "double"
                    || type == "int8" || type == "uint8"
                    || type == "int16" || type == "uint16"
                    || type == "int32" || type == "uint32"
                    || type == "int64" || type == "uint64")
                {
                    if (body.at(name) < minValue)
                    {
                        obj["error"] = "Field '" + name + "' should be greater or equal to " + std::to_string(minValue);
                        obj["status"] = 400;
                        return obj;
                    }
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
}
