//
// Created by allan on 20/05/2025.
//

#include "../../../include/mantis/core/tables/sys_tables.h"
#include "../../../include/mantis/core/database.h"
#include "../../../include/mantis/app/app.h"
#include "../../../include/mantis/utils/utils.h"
#include "../../../include/mantis/core/router.h"

#define __file__ "core/tables/sys_tables.cpp"

namespace mantis
{
    SysTablesUnit::SysTablesUnit(const std::string& tableName,
                                 const std::string& tableId,
                                 const std::string& tableType)
        : TableUnit(tableName, tableId, tableType)
    {
    }

    bool SysTablesUnit::setupRoutes()
    {
        if (m_tableName.empty() && m_routeName.empty()) return false;

        const auto path = m_routeName.empty() ? m_tableName : m_routeName;
        const auto basePath = "/api/v1/" + path;

        try
        {
            // Fetch All Records
            Log::debug("Adding GET Request for table '{}'", m_tableName);
            MantisApp::instance().http().Get(
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
            MantisApp::instance().http().Get(
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

            // Add Record
            Log::debug("Adding POST Request for table '{}'", m_tableName);
            MantisApp::instance().http().Post(
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
            MantisApp::instance().http().Patch(
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
            MantisApp::instance().http().Delete(
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

            // Add Record
            Log::debug("Adding POST Request for table '{}/auth-with-password'", m_routeName);
            MantisApp::instance().http().Post(
                basePath + "/auth-with-password",
                [this](const Request& req, Response& res, Context& ctx) -> void
                {
                    authWithEmailAndPassword(req, res, ctx);
                }
            );

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

    void SysTablesUnit::fetchRecord(const Request& req, Response& res, Context& ctx)
    {
        // Response Object
        json response;

        // Get the path param ID value, return 400 error if its invalid
        const auto id = req.path_params.at("id");
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
            // Read record by 'id' above and return record if its found
            if (const auto resp = read(id, json::object()); resp.has_value())
            {
                response["status"] = 200;
                response["error"] = "";
                response["data"] = resp.value();

                res.set_content(response.dump(), "application/json");
                res.status = 200;
                return;
            }

            // If no record is available, return 404
            response["status"] = 404;
            response["error"] = "Item Not Found";
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 404;
            return;
        }
        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["error"] = e.what();
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 500;
            return;
        }

        catch (...)
        {
            response["status"] = 500;
            response["error"] = "Unknown Error";
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 500;
            return;
        }
    }

    void SysTablesUnit::fetchRecords(const Request& req, Response& res, Context& ctx)
    {
        json response;
        try
        {
            // Fetch all records in the database
            auto items = list(json::object());
            response["data"] = items;
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

    void SysTablesUnit::createRecord(const Request& req, Response& res, Context& ctx)
    {
        json body, response;

        // Parse request body to JSON Object, return an error if it fails
        try { body = json::parse(req.body); }
        catch (const std::exception& e)
        {
            response["status"] = 400;
            response["error"] = e.what();
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }

        Log::debug("Create table, data = {}", body.dump());

        // Create validator method
        auto validateRequestBody = [&]() -> bool
        {
            if (trim(body.value("name", "")).empty())
            {
                response["status"] = 400;
                response["error"] = "Table name is required";
                response["data"] = json::object();

                res.status = 400;
                res.set_content(response.dump(), "application/json");
                return false;
            }

            // Check that the type is either a view|base|auth type
            auto type = trim(body.value("type", ""));
            toLowerCase(type);
            if (!(type == "view" || type == "base" || type == "auth"))
            {
                response["status"] = 400;
                response["error"] = "Table type should be either 'base', 'view', or 'auth'";
                response["data"] = json::object();

                res.status = 400;
                res.set_content(response.dump(), "application/json");
                return false;
            }

            // If the table type is of view type, check that the SQL is passed in ...
            if (type == "view")
            {
                if (trim(body.value("sql", "")).empty())
                {
                    response["status"] = 400;
                    response["error"] = "Table of view type require an SQL Statement.";
                    response["data"] = json::object();

                    res.status = 400;
                    res.set_content(response.dump(), "application/json");
                    return false;
                }
            }
            else
            {
                // Check fields if any is added
                for (const auto& field : body.value("fields", std::vector<json>()))
                {
                    const auto name = trim(field.value("name", ""));
                    if (name.empty())
                    {
                        response["status"] = 400;
                        response["error"] = "One of the fields is missing a valid name";
                        response["data"] = json::object();

                        res.status = 400;
                        res.set_content(response.dump(), "application/json");
                        return false;
                    }

                    const auto field_type = trim(field.value("type", ""));
                    if (field_type.empty())
                    {
                        response["status"] = 400;
                        response["error"] = "Field type '" + field_type + "' for '" + name +
                            "' is not a valid type";
                        response["data"] = json::object();

                        res.status = 400;
                        res.set_content(response.dump(), "application/json");
                        return false;
                    }
                    if ()
                }
            }

            return true;
        };

        // Validate JSON body
        if (!validateRequestBody()) return;

        // Invoke create table method, return the result
        auto resp = create(body, json::object());
        if (const auto err = resp.value("error", json::object()); !err.empty())
        {
            int status = resp.value("status", 500);
            response["status"] = status;
            response["error"] = err;
            response["data"] = json::object();

            res.status = status;
            res.set_content(response.dump(), "application/json");

            Log::critical("Failed to create table, reason: {}", resp.dump());
            return;
        }

        response["status"] = 201;
        response["error"] = "";
        response["data"] = resp.at("data");

        res.status = 201;
        res.set_content(response.dump(), "application/json");
    }

    void SysTablesUnit::updateRecord(const Request& req, Response& res, Context& ctx)
    {
        Log::trace("Updating table, endpoint {}", req.path);

        json body, response;
        // Extract request ID and check that it's not empty
        const auto id = req.path_params.at("id");

        // For empty IDs, return 400, BAD REQUEST
        if (id.empty())
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Table ID is required!";

            res.status = 400;
            res.set_content(response.dump(), "application/json");
            return;
        }

        try
        {
            // Try parsing the request body, may checkMinValueFunc ...
            body = json::parse(req.body);
        }
        catch (const std::exception& e)
        {
            // Return 400 BAD REQUEST for any parse errors
            response["status"] = 400;
            response["error"] = e.what();
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }

        // Check that record exists before we continue ...
        if (!recordExists(id))
        {
            response["status"] = 404;
            response["data"] = json::object();
            response["error"] = "Record with id " + id + " was not found.";

            res.status = 404;
            res.set_content(response.dump(), "application/json");
            return;
        }

        // Try creating the record, if it checkMinValueFuncs, return the error
        auto respObj = update(id, body, json::object());
        if (const auto err = respObj.value("error", ""); !err.empty())
        {
            int status = respObj.value("status", 500);
            response["status"] = status;
            response["error"] = err;
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = status;

            Log::critical("Failed to update table, id = {}, reason: {}", id, respObj.dump());
            return;
        }

        // Get the data, for auth types, redact the password information
        // it's not useful data to return in the response despite being hashed
        const auto record = respObj.at("data");
        Log::trace("Record update successful: {}", record.dump());

        // Return the added record + the system generated fields
        response["status"] = 200;
        response["error"] = "";
        response["data"] = record;

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    }

    void SysTablesUnit::deleteRecord(const Request& req, Response& res, Context& ctx)
    {
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
            // If remove returns false, something didn't go right!
            if (const auto resp = remove(id, json::object()); !resp)
            {
                response["status"] = 500;
                response["error"] = "Could not delete record";
                response["data"] = json::object();

                res.set_content(response.dump(), "application/json");
                res.status = 500;
                return;
            }
        }
        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["error"] = e.what();
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 500;
            return;
        }

        res.set_content("", "application/json");
        res.status = 204;
    }

    void SysTablesUnit::authWithEmailAndPassword(const Request& req, Response& res, Context& ctx)
    {
        Log::trace("Auth on record, endpoint {}", req.path);

        json body, response;
        try { body = json::parse(req.body); }
        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["data"] = json::object();
            response["error"] = "Could not parse request body! ";

            res.status = 500;
            res.set_content(response.dump(), "application/json");

            Log::critical("Could not parse request body! Reason: {}", e.what());
            return;
        }

        // Get email & password values
        const auto email = body.value("email", "");
        const auto password = body.value("password", "");

        // We expect, the user email and password to be passed in
        if (email.length() < 5)
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Admin user `email` is missing!";

            res.status = 400;
            res.set_content(response.dump(), "application/json");
            return;
        }

        if (password.length() < 8)
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Admin user `password` is missing!";

            res.status = 400;
            res.set_content(response.dump(), "application/json");
            return;
        }

        try
        {
            // Find user with an email passed in ....
            const auto sql = MantisApp::instance().db().session();

            soci::row r;
            const auto query = "SELECT * FROM " + m_tableName + " WHERE email = :email LIMIT 1;";
            *sql << query, soci::use(email), soci::into(r);

            if (!sql->got_data())
            {
                response["status"] = 404;
                response["data"] = json::object();
                response["error"] = "No user found matching given email/password combination.";

                res.status = 404;
                res.set_content(response.dump(), "application/json");

                Log::debug("[No DB Match] No user found matching given email/password combination. {}",
                           response.dump());
                return;
            }

            // Extract user password value
            const auto db_password = r.get<std::string>("password");
            const auto resp = verifyPassword(password, db_password);
            if (resp.value("verified", false))
            {
                // Create JWT Token and return to the user ...
                auto user = parseDbRowToJson(r);
                user.erase("password"); // Remove password field

                // C
                const json claims{{"id", user.at("id").get<std::string>()}, {"table", m_tableName}};
                const auto obj = JWT::createJWTToken(claims, MantisApp::jwtSecretKey());
                if (const auto err = obj.at("error").get<std::string>(); !err.empty())
                {
                    response["status"] = 500;
                    response["data"] = "";
                    response["error"] = err;

                    res.status = 500;
                    res.set_content(response.dump(), "application/json");

                    Log::info("Error creating JWT token: {}", response.dump());

                    return;
                }

                json data;
                data["user"] = user;
                data["token"] = obj.at("token").get<std::string>();

                response["status"] = 200;
                response["data"] = data;
                response["error"] = "";

                res.status = 200;
                res.set_content(response.dump(), "application/json");
                // Log::info("Login Successful, user: {}", response.dump());
                return;
            }

            response["status"] = 404;
            response["data"] = json::object();
            response["error"] = "No user found matching given email/password combination.";

            res.status = 404;
            res.set_content(response.dump(), "application/json");

            Log::warn("No user found for given email/password combination. Reason: {}",
                    resp.value("error", ""));
        }
        catch (std::exception& e)
        {
            Log::critical("Error on admin login: {}", e.what());
            response["status"] = 500;
            response["data"] = json::object();
            response["error"] = e.what();

            res.status = 500;
            res.set_content(response.dump(), "application/json");

            Log::critical("Error Processing Request: {}", e.what());
        }
        catch (...)
        {
            response["status"] = 500;
            response["data"] = json::object();
            response["error"] = "Internal Server Error";

            res.status = 500;
            res.set_content(response.dump(), "application/json");
            Log::critical("Error on admin login: Unknown Error");
        }
    }

    bool SysTablesUnit::hasAccess(const Request& req, Response& res, Context& ctx)
    {
        // Get the auth var from the context, resort to empty object if it's not set.
        auto auth = *ctx.get<json>("auth").value_or(new json{json::object()});

        // fetch auth token
        const auto& token = auth.value("token", "");
        if (token.empty())
        {
            json response;
            response["status"] = 403;
            response["data"] = json::object();
            response["error"] = "Auth token missing";

            res.status = 403;
            res.set_content(response.dump(), "application/json");

            DUMP_RESPONSE();
            return REQUEST_HANDLED;
        }

        // Expand logged user if token is present
        const auto resp = JWT::verifyJWTToken(token, MantisApp::jwtSecretKey());
        Log::trace("RESP: {}", resp.dump());
        if (!resp.value("verified", false) || !resp.value("error", "").empty())
        {
            json response;
            response["status"] = 403;
            response["data"] = json::object();
            response["error"] = resp.value("error", "");

            res.status = 403;
            res.set_content(response.dump(), "application/json");
            DUMP_RESPONSE();
            return REQUEST_HANDLED;
        }

        // Extract and verify that the id and table data is provided, else,
        // return an error
        const auto _id = resp.value("id", "");
        const auto _table = resp.value("table", "");
        Log::trace("After Auth: id = {}, table = {}", _id, _table);

        if (_id.empty() || _table.empty())
        {
            json response;
            response["status"] = 403;
            response["data"] = json::object();
            response["error"] = "Auth token missing user id or table name";

            res.status = 403;
            res.set_content(resp.dump(), "application/json");
            DUMP_RESPONSE();
            return REQUEST_HANDLED;
        }

        // Query for user with given ID, this info will be populated to the
        // expression evaluator args as well as available through
        // the session context, queried by:
        //  ` ctx.get<json>("auth").value("id", ""); // returns the user ID
        //  ` ctx.get<json>("auth").value("name", ""); // returns the user's name
        auto sql = MantisApp::instance().db().session();
        soci::row r;
        std::string query = "SELECT * FROM __admins WHERE id = :id LIMIT 1";
        *sql << query, soci::use(_id), soci::into(r);

        // Return 404 if user was not found
        if (!sql->got_data())
        {
            json response;
            response["status"] = 404;
            response["data"] = json::object();
            response["error"] = "Auth id was not found.";

            res.status = 404;
            res.set_content(resp.dump(), "application/json");

            DUMP_RESPONSE();
            return REQUEST_HANDLED;
        }

        // Populate the auth object with additional data from the database
        // remove `password` field if available
        auto user = parseDbRowToJson(r);
        auth["type"] = "user";
        auth["id"] = _id;
        auth["table"] = _table;

        // Populate auth obj with user details ...
        for (const auto& [key, value] : user.items())
        {
            auth[key] = value;
        }

        // Remove password field
        auth.erase("password");

        // Update context data
        ctx.set("auth", auth);

        const auto table_name = auth.value("table", "");
        Log::trace("Auth table is: {}", table_name);
        // Check if user is logged in as Admin
        if (table_name == "__admins")
        {
            Log::trace("Logged in as Admin!");
            // If logged in as admin, grant access
            // Admins get unconditional data access
            return REQUEST_PENDING;
        }

        // User was not an admin, lets return access denied error
        json response;
        response["status"] = 403;
        response["data"] = json::object();
        response["error"] = "Admin auth required to access this resource.";

        res.status = 403;
        res.set_content(response.dump(), "application/json");
        DUMP_RESPONSE();
        return REQUEST_HANDLED;
    }
} // mantis
