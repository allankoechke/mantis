#include "../../../include/mantis/core/tables/sys_tables.h"
#include "../../../include/mantis/core/database_mgr.h"
#include "../../../include/mantis/mantisbase.h"
#include "../../../include/mantis/utils/utils.h"
#include "../../../include/mantis/core/router.h"
#include "../../../include/mantis/core/jwt_mgr.h"

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
        TRACE_CLASS_METHOD()

        if (m_tableName.empty() && m_routeName.empty()) return false;

        const auto path = m_routeName.empty() ? m_tableName : m_routeName;
        const auto basePath = "/api/v1/" + path;

        try
        {
            // Fetch All Records
            logger::debug("Creating route: [{:>6}] {}", "GET", basePath);
            MantisBase::instance().http().Get(
                basePath,
                [this](MantisRequest& req, MantisResponse& res)-> void
                {
                    fetchRecords(req, res);
                },
                {
                    [](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return getAuthToken(req, res);
                    },
                    [this](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return hasAccess(req, res);
                    }
                }
            );

            // Fetch Single Record
            logger::debug("Creating route: [{:>6}] {}/:id", "GET", basePath);
            MantisBase::instance().http().Get(
                basePath + "/:id",
                [this](MantisRequest& req, MantisResponse& res)-> void
                {
                    fetchRecord(req, res);
                },
                {
                    [this](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return getAuthToken(req, res);
                    },
                    [this](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return hasAccess(req, res);
                    }
                }
            );

            // Add Record
            logger::debug("Creating route: [{:>6}] {}", "POST", basePath);
            MantisBase::instance().http().Post(
                basePath,
                [this](MantisRequest& req, MantisResponse& res, const MantisContentReader& reader)-> void
                {
                    createRecord(req, res, reader);
                },
                {
                    [this](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return getAuthToken(req, res);
                    },
                    [this](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return hasAccess(req, res);
                    }
                }
            );

            // Update Record
            logger::debug("Creating route: [{:>6}] {}/:id", "PATCH", basePath);
            MantisBase::instance().http().Patch(
                basePath + "/:id",
                [this](MantisRequest& req, MantisResponse& res, const MantisContentReader& reader)-> void
                {
                    updateRecord(req, res, reader);
                },
                {
                    [this](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return getAuthToken(req, res);
                    },
                    [this](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return hasAccess(req, res);
                    }
                }
            );

            // Delete Record
            logger::debug("Creating route: [{:>6}] {}/:id", "DELETE", basePath);
            MantisBase::instance().http().Delete(
                basePath + "/:id",
                [this](MantisRequest& req, MantisResponse& res)-> void
                {
                    deleteRecord(req, res);
                },
                {
                    [this](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return getAuthToken(req, res);
                    },
                    [this](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return hasAccess(req, res);
                    }
                }
            );

            // Add Record
            logger::debug("Creating route: [{:>6}] {}/auth-with-password", "POST", basePath);
            MantisBase::instance().http().Post(
                basePath + "/auth-with-password",
                [this](MantisRequest& req, MantisResponse& res) -> void
                {
                    authWithEmailAndPassword(req, res);
                }
            );

            return true;
        }

        catch (const std::exception& e)
        {
            logger::critical("Failed to create routes for table '{}' of '{}' type: {}",
                          m_tableName, m_tableType, e.what());
            return false;
        }

        catch (...)
        {
            logger::critical("Failed to create routes for table '{}' of '{}' type: {}",
                          m_tableName, m_tableType, "Unknown Error!");
            return false;
        }
    }

    void SysTablesUnit::fetchRecord(MantisRequest& req, MantisResponse& res)
    {
        TRACE_CLASS_METHOD()

        // Response Object
        json response;

        // Get the path param ID value, return 400 error if its invalid
        const auto id = req.getPathParamValue("id");
        if (id.empty())
        {
            response["status"] = 400;
            response["error"] = "Record ID is required";
            response["data"] = json::object();

            res.sendJson(400, response);
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

                res.sendJson(200, response);
                return;
            }

            // If no record is available, return 404
            response["status"] = 404;
            response["error"] = "Item Not Found";
            response["data"] = json::object();

            res.sendJson(404, response);
        }
        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["error"] = e.what();
            response["data"] = json::object();

            res.sendJson(500, response);
        }

        catch (...)
        {
            response["status"] = 500;
            response["error"] = "Unknown Error";
            response["data"] = json::object();

            res.sendJson(500, response);
        }
    }

    void SysTablesUnit::fetchRecords(MantisRequest& req, MantisResponse& res)
    {
        TRACE_CLASS_METHOD()

        json response;
        try
        {
            // Fetch all records in the database
            auto items = list(json::object());
            response["data"] = items;
            response["status"] = 200;
            response["error"] = "";

            res.sendJson(200, response);
        }

        catch (const std::exception& e)
        {
            response["data"] = json::array();
            response["status"] = 500;
            response["error"] = e.what();

            res.sendJson(500, response);
        }

        catch (...)
        {
            response["data"] = json::array();
            response["status"] = 500;
            response["error"] = "Internal Server Error";

            res.sendJson(500, response);
        }
    }

    void SysTablesUnit::createRecord(MantisRequest& req, MantisResponse& res, const MantisContentReader& reader)
    {
        TRACE_CLASS_METHOD()

        json body, response;

        // Handle as JSON/regular body
        std::string body_str;
        reader([&](const char* data, const size_t data_length) -> bool
        {
            body_str.append(data, data_length);
            return true;
        });

        // Parse request body to JSON Object, return an error if it fails
        try
        {
            if (body_str.empty()) body = json::object();
            else body = json::parse(body_str);
        }
        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["error"] = std::format("Error parsing Table Schema: {}", e.what());
            response["data"] = json::object();

            res.sendJson(500, response);
            return;
        }

        // Validate JSON body
        const auto& validate_res = validateTableSchema(body);
        if (validate_res.has_value())
        {
            res.sendJson(validate_res.value()["status"].get<int>(), validate_res.value());
            logger::critical("Failed to create table, reason: {}", validate_res.value().dump());
            return;
        }

        // Invoke create table method, return the result
        auto resp = create(body, json::object());
        if (const auto err = resp.value("error", json::object()); !err.empty())
        {
            int status = resp.value("status", 500);
            response["status"] = status;
            response["error"] = err;
            response["data"] = json::object();

            res.sendJson(status, response);

            logger::critical("Failed to create table, reason: {}", resp.dump());
            return;
        }

        response["status"] = 201;
        response["error"] = "";
        response["data"] = resp.at("data");

        res.sendJson(201, response);
    }

    void SysTablesUnit::updateRecord(MantisRequest& req, MantisResponse& res, const MantisContentReader& reader)
    {
        TRACE_CLASS_METHOD()

        json body, response;
        // Extract request ID and check that it's not empty
        const auto id = req.getPathParamValue("id");

        // For empty IDs, return 400, BAD REQUEST
        if (id.empty())
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Table ID is required!";

            res.sendJson(400, response);
            return;
        }

        // Handle as JSON/regular body
        std::string body_str;
        reader([&](const char* data, const size_t data_length) -> bool
        {
            body_str.append(data, data_length);
            return true;
        });

        // Parse request body to JSON Object, return an error if it fails
        try
        {
            if (body_str.empty()) body = json::object();
            else body = json::parse(body_str);
        }
        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["error"] = std::format("Error parsing table schema: {}", e.what());
            response["data"] = json::object();

            res.sendJson(500, response);
            return;
        }

        // TODO Validate update body ...

        // Check that record exists before we continue ...
        if (!recordExists(id))
        {
            response["status"] = 404;
            response["data"] = json::object();
            response["error"] = std::format("Record with id = `{}` was not found.", id);

            res.sendJson(404, response);
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

            res.sendJson(status, response);

            logger::critical("Failed to update table, id = {}, reason: {}", id, respObj.dump());
            return;
        }

        // Get the data, for auth types, redact the password information
        // it's not useful data to return in the response despite being hashed
        const auto record = respObj.at("data");

        // Return the added record + the system generated fields
        response["status"] = 200;
        response["error"] = "";
        response["data"] = record;

        res.sendJson(200, response);
    }

    void SysTablesUnit::deleteRecord(MantisRequest& req, MantisResponse& res)
    {
        TRACE_CLASS_METHOD()

        const auto id = req.getPathParamValue("id");
        json response;
        if (id.empty())
        {
            response["status"] = 400;
            response["error"] = "Record ID is required";
            response["data"] = json::object();

            res.sendJson(400, response);
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

                res.sendJson(500, response);
                return;
            }
        }
        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["error"] = e.what();
            response["data"] = json::object();

            res.sendJson(500, response);
            return;
        }

        res.sendEmpty();
    }

    void SysTablesUnit::authWithEmailAndPassword(MantisRequest& req, MantisResponse& res)
    {
        TRACE_CLASS_METHOD()

        json body, response;
        try { body = json::parse(req.getBody()); }
        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["data"] = json::object();
            response["error"] = "Could not parse request body! ";

            res.sendJson(500, response);
            return;
        }

        // Get email & password values
        const auto email = (body.contains("email") && !body["email"].is_null()) ? body.value("email", "") : "";
        const auto password = (body.contains("password") && !body["password"].is_null())
                                  ? body.value("password", "")
                                  : "";

        // We expect, the user email and password to be passed in
        if (email.length() < 5)
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Admin user `email` is missing!";


            res.sendJson(400, response);
            res.sendJson(400, response);
            return;
        }

        if (password.length() < 8)
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Admin user `password` is missing!";

            res.sendJson(400, response);
            return;
        }

        try
        {
            // Find user with an email passed in ....
            const auto sql = MantisBase::instance().db().session();

            soci::row r;
            const auto query = "SELECT * FROM " + m_tableName + " WHERE email = :email LIMIT 1;";
            *sql << query, soci::use(email), soci::into(r);

            if (!sql->got_data())
            {
                response["status"] = 404;
                response["data"] = json::object();
                response["error"] = "No user found matching given email/password combination.";

                res.sendJson(404, response);
                return;
            }

            // Extract user password value
            const auto db_password = r.get<std::string>("password");
            const auto p_verified = verifyPassword(password, db_password);
            if (!p_verified)
            {
                response["status"] = 404;
                response["data"] = json::object();
                response["error"] = "No user found matching given email/password combination.";

                res.sendJson(404, response);
                logger::warn("No user found for given email/password combination.");
                return;
            }

            // Create JWT Token and return to the user ...
            auto user = parseDbRowToJson(r);
            user.erase("password"); // Remove password field

            const json claims{
                {"id", user.at("id").get<std::string>()},
                {"table", m_tableName}
            };

            const auto token = JwtUnit::createJWTToken(claims, 60 * 60); // 1hr
            json data;
            data["user"] = user;
            data["token"] = token;

            response["status"] = 200;
            response["data"] = data;
            response["error"] = "";

            res.sendJson(200, response);
        }
        catch (std::exception& e)
        {
            logger::critical("Error on admin login: {}", e.what());
            response["status"] = 500;
            response["data"] = json::object();
            response["error"] = e.what();

            res.sendJson(500, response);
            logger::critical("Error Processing Request: {}", e.what());
        }
    }

    bool SysTablesUnit::hasAccess(MantisRequest& req, MantisResponse& res)
    {
        TRACE_CLASS_METHOD()

        // Get the auth var from the context, resort to empty object if it's not set.
        auto auth = req.getOr<json>("auth", json::object());

        // fetch auth token
        const auto& token = auth.value("token", "");
        if (token.empty())
        {
            json response;
            response["status"] = 403;
            response["data"] = json::object();
            response["error"] = "Auth token missing";

            res.sendJson(403, response);
            return REQUEST_HANDLED;
        }

        // Expand logged user if token is present
        const auto resp = JwtUnit::verifyJwtToken(token);
        if (!resp.value("verified", false) || !resp.at("error").get<std::string>().empty())
        {
            json response;
            response["status"] = 403;
            response["data"] = json::object();
            response["error"] = resp.value("error", "");

            res.sendJson(403, response);
            return REQUEST_HANDLED;
        }

        // Extract and verify that the id and table data is provided, else,
        // return an error
        const auto auth_user_id = resp.value("id", "");
        const auto auth_table = resp.value("table", "");

        if (auth_user_id.empty() || auth_table.empty())
        {
            json response;
            response["status"] = 403;
            response["data"] = json::object();
            response["error"] = "Auth token missing user id or table name";

            res.sendJson(403, response);
            return REQUEST_HANDLED;
        }

        // Query for user with given ID, this info will be populated to the
        // expression evaluator args as well as available through
        // the session context, queried by:
        //  ` req.get<json>("auth").value("id", ""); // returns the user ID
        //  ` req.get<json>("auth").value("name", ""); // returns the user's name
        auto sql = MantisBase::instance().db().session();

        soci::row admin_row;
        *sql << "SELECT id, email, created, updated FROM __admins WHERE id = :id LIMIT 1",
        soci::use(auth_user_id), soci::into(admin_row);

        // Return 404 if user was not found
        if (!sql->got_data())
        {
            json response;
            response["status"] = 404;
            response["data"] = json::object();
            response["error"] = "Auth id was not found.";

            res.sendJson(404, response);
            return REQUEST_HANDLED;
        }

        try
        {
            // Populate the auth object with additional data from the database
            // remove `password` field if available
            json user;
            user["id"] = admin_row.get<std::string>(0);
            user["email"] = admin_row.get<std::string>(1);

            // Handle date parsing, SQLITE uses `string` type
            if (admin_row.get_properties(2).get_db_type() == soci::db_date)
            {
                auto t = admin_row.get<std::tm>(2);
                user["created"] = tmToStr(t);

                t = admin_row.get<std::tm>(3);
                user["updated"] = tmToStr(t);
            }
            else
            {
                user["created"] = admin_row.get<std::string>(2);
                user["updated"] = admin_row.get<std::string>(3);
            }

            // Enrich auth object with auth information
            auth["type"] = "user";
            auth["id"] = auth_user_id;
            auth["table"] = auth_table;

            // Populate auth obj with user details ...
            for (const auto& [key, value] : user.items())
            {
                auth[key] = value;
            }
        }
        catch (const std::exception& e)
        {
            json response;
            response["status"] = 500;
            response["data"] = json::object();
            response["error"] = "Internal Server Error while parsing user record!";

            res.sendJson(500, response);
            logger::critical("Error parsing logged user: {}", e.what());
            return REQUEST_HANDLED;
        }

        // Update context data
        req.set("auth", auth);

        const auto table_name = auth.value("table", "");
        // logger::trace("Auth table is: {}", table_name);
        // Check if user is logged in as Admin
        if (table_name == "__admins")
        {
            // If logged in as admin, grant access
            // Admins get unconditional data access
            return REQUEST_PENDING;
        }

        // User was not an admin, lets return access denied error
        json response;
        response["status"] = 403;
        response["data"] = json::object();
        response["error"] = "Admin auth required to access this resource.";

        res.sendJson(403, response);
        return REQUEST_HANDLED;
    }
} // mantis
