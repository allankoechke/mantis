#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../include/mantis/utils/utils.h"
#include "../../include/mantis/core/jwt.h"
#include "../../include/mantis/core/router.h"

#define __file__ "core/tables/tables_auth.cpp"

namespace mantis
{
    void TableUnit::authWithEmailAndPassword(MantisRequest& req, MantisResponse& res)
    {
        TRACE_CLASS_METHOD();

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

        // We expect, the user email and password to be passed in
        if (!body.contains("email") || body.value("email", "").length() < 5)
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "User email is missing!";

            res.sendJson(400, response);
            return;
        }

        if (!body.contains("password") || body.value("password", "").length() < 8)
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "User password is missing!";

            res.sendJson(400, response);
            return;
        }

        try
        {
            const auto email = body.value("email", "");
            const auto password = body.value("password", "");

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

                res.sendJson(404, response);
                return;
            }

            // Extract user password value
            const auto db_password = r.get<std::string>("password");

            // Verify user password
            if (const auto p_verified = verifyPassword(password, db_password); !p_verified)
            {
                response["status"] = 404;
                response["data"] = json::object();
                response["error"] = "No user found matching given email/password combination.";

                res.sendJson(404, response);
                Log::warn("No user found for given email/password combination");
            }

            // Create JWT Token and return to the user ...
            auto user = parseDbRowToJson(r);
            user.erase("password"); // Remove password field

            // Create user claims to be added to the token
            const json claims{
                {"id", user.at("id").get<std::string>()},
                {"table", m_tableName}
            };

            try
            {
                // Create a user token
                const auto token = JwtUnit::createJWTToken(claims);

                json data;
                data["user"] = user;
                data["token"] = token;
                response["status"] = 200;
                response["data"] = data;
                response["error"] = "";

                res.sendJson(200, response);
                return;
            }
            catch (const std::exception& e)
            {
                response["status"] = 500;
                response["data"] = json::object();
                response["error"] = std::format("Failed to create token: {}", e.what());

                res.sendJson(500, response);
                return;
            }
        }
        catch (std::exception& e)
        {
            response["status"] = 500;
            response["data"] = json::object();
            response["error"] = e.what();

            res.sendJson(500, response);
            Log::critical("Error Processing Request: {}", e.what());
        }
        catch (...)
        {
            response["status"] = 500;
            response["data"] = json::object();
            response["error"] = "Internal Server Error";

            res.sendJson(500, response);
            Log::critical("Internal Server Error");
        }
    }

    void TableUnit::resetPassword(MantisRequest& req, MantisResponse& res)
    {
        TRACE_CLASS_METHOD();

        [[maybe_unused]] auto sql = MantisApp::instance().db().session();

        Log::trace("Resetting password on record, endpoint {}", req.getPath());
        //res.sendJson(200, req);
    }

    bool TableUnit::getAuthToken(MantisRequest& req, [[maybe_unused]] MantisResponse& res)
    {
        // If we have an auth header, extract it into the ctx, else
        // add a guest user type. The auth if present, should have
        // the user id, auth table, etc.
        json auth;
        auth["type"] = "guest"; // or 'user'
        auth["token"] = nullptr;
        auth["id"] = nullptr;
        auth["table"] = nullptr;

        if (req.hasHeader("Authorization"))
        {
            const auto token = req.getBearerTokenAuth();
            auth["token"] = trim(token);
            auth["type"] = "user";
        }

        // Update the context
        req.set("auth", auth);
        return REQUEST_PENDING;
    }

    bool TableUnit::hasAccess(MantisRequest& req, MantisResponse& res)
    {
        TRACE_CLASS_METHOD();

        // Get the auth var from the context, resort to empty object if it's not set.
        auto auth = req.getOr<json>("auth", json::object());

        Log::trace("Auth Obj: `{}`", auth.dump());

        auto method = req.getMethod();
        if (!(method == "GET"
            || method == "POST"
            || method == "PATCH"
            || method == "DELETE"))
        {
            json response;
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Unsupported method";

            res.sendJson(400, response);
            return REQUEST_HANDLED;
        }

        // Store rule, depending on the request type
        std::string rule = method == "GET"
                               ? (req.hasPathParams() ? m_listRule : m_getRule)
                               : method == "POST"
                               ? m_addRule
                               : method == "PATCH"
                               ? m_updateRule
                               : m_deleteRule;

        Log::trace("Rule: `{}`", rule);

        // Remove whitespaces
        rule = trim(rule);

        // Token map variables for evaluation
        TokenMap vars;

        // Expand logged user if token is present and query user information if it exists
        if (auth.contains("token") && !auth["token"].is_null() && !auth["token"].empty())
        {
            const auto token = auth.at("token").get<std::string>();

            // If token validation worked, lets get data from database
            if (const auto resp = JwtUnit::verifyJwtToken(token); resp.at("verified").get<bool>())
            {
                Log::trace("Token Verified: `{}`", token);
                const auto user_id = resp.at("id").get<std::string>();
                const auto user_table = resp.at("table").get<std::string>();

                // Query for user with given ID, this info will be populated to the
                // expression evaluator args as well as available through
                // the session context, queried by:
                //  ` req.get<json>("auth").value("id", ""); // returns the user ID
                //  ` req.get<json>("auth").value("name", ""); // returns the user's name
                auto sql = MantisApp::instance().db().session();
                std::string query = "SELECT * FROM " + user_table + " WHERE id = :id LIMIT 1";

                soci::row user_row;
                *sql << query, soci::use(user_id), soci::into(user_row);

                // Let's only populate TokenVars for now, no returning
                if (sql->got_data())
                {
                    // Populate the auth object with additional data from the database
                    // remove `password` field if available
                    auto user = user_table == "__admins"
                                    ? parseDbRowToJson(user_row, MantisApp::instance().router().adminTableFields)
                                    : parseDbRowToJson(user_row);

                    // Populate the `auth` object
                    auth["type"] = "user";
                    auth["id"] = user_id;
                    auth["table"] = user_table;

                    // Populate auth obj with user details ...
                    for (const auto& [key, value] : user.items())
                    {
                        auth[key] = value;
                    }

                    // Remove password field
                    auth.erase("password");

                    // Update context data
                    req.set("auth", auth);

                    // Add `auth` data to the TokenMap
                    vars["auth"] = MantisApp::instance().evaluator().jsonToTokenMap(auth);
                }
            }
        }

        // Request Token Map
        TokenMap reqMap;
        reqMap["remoteAddr"] = req.getRemoteAddr();
        reqMap["remotePort"] = req.getRemotePort();
        reqMap["localAddr"] = req.getLocalAddr();
        reqMap["localPort"] = req.getLocalPort();

        try
        {
            if (req.getMethod() == "POST" && !req.getBody().empty()) // TODO handle formdata
            {
                // Parse request body and add it to the request TokenMap
                auto request = json::parse(req.getMethod());
                reqMap["body"] = MantisApp::instance().evaluator().jsonToTokenMap(request);
            }
        }
        catch (...)
        {
        }

        // Add the request map to the vars
        vars["req"] = reqMap;

        // If the rule is empty, enforce admin authorization
        if (rule.empty())
        {
            // Check if user is logged in as Admin
            if (auth.contains("table")  // Has `table` key
                && !auth["table"].is_null() // `table` value is not null
                && auth.at("table").get<std::string>() == "__admins"
            ) // Check table value not null
            {
                // If logged in as admin, grant access
                // Admins get unconditional data access
                return REQUEST_PENDING;
            }

            Log::trace("Table: `{}`", auth.at("table").get<std::string>());

            // User was not an admin, lets return access denied error
            json response;
            response["status"] = 403;
            response["data"] = json::object();
            response["error"] = "Admin auth required to access this resource.";

            res.sendJson(403, response);
            return REQUEST_HANDLED;
        }

        Log::trace("Expression Rule = {}", rule);

        // If expression evaluation returns true, lets return allowing execution
        // continuation. Else, we'll craft an error response.
        if (MantisApp::instance().evaluator().evaluate(rule, vars))
            return REQUEST_PENDING; // Proceed to next middleware

        // Evaluation yielded false, return generic access denied error
        json response;
        response["status"] = 403;
        response["data"] = json::object();
        response["error"] = "Access denied!";

        res.sendJson(403, response);
        return REQUEST_HANDLED;
    }
}
