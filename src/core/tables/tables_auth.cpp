//
// Created by allan on 07/06/2025.
//

#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../../include/mantis/utils/utils.h"
#include "mantis/core/router.h"

#define __file__ "core/tables/tables_auth.cpp"

namespace mantis
{
    void TableUnit::authWithEmailAndPassword(const Request& req, Response& res, Context& ctx)
    {
        TRACE_CLASS_METHOD();

        json body, response;
        try { body = json::parse(req.body); }
        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["data"] = json::object();
            response["error"] = "Could not parse request body! ";

            res.status = 500;
            res.set_content(response.dump(), "application/json");
            return;
        }

        // We expect, the user email and password to be passed in
        if (!body.contains("email") || body.value("email", "").length() < 5)
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "User email is missing!";

            res.status = 400;
            res.set_content(response.dump(), "application/json");
            return;
        }

        if (!body.contains("password") || body.value("password", "").length() < 8)
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "User password is missing!";

            res.status = 400;
            res.set_content(response.dump(), "application/json");
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

                res.status = 404;
                res.set_content(response.dump(), "application/json");
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
                    response["data"] = json::object();
                    response["error"] = err;

                    res.status = 500;
                    res.set_content(response.dump(), "application/json");
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

            Log::critical("Internal Server Error");
        }
    }

    void TableUnit::resetPassword(const Request& req, Response& res, Context& ctx)
    {
        TRACE_CLASS_METHOD();

        [[maybe_unused]] auto sql = MantisApp::instance().db().session();

        Log::trace("Resetting password on record, endpoint {}", req.path);
        res.status = 200;
        res.set_content(req.body, "application/json");
    }

    bool TableUnit::getAuthToken(const Request& req, [[maybe_unused]] Response& res, Context& ctx)
    {
        // If we have an auth header, extract it into the ctx, else
        // add a guest user type. The auth if present, should have
        // the user id, auth table, etc.
        json auth;
        auth["type"] = "guest"; // or 'user'
        auth["token"] = nullptr;
        auth["id"] = nullptr;
        auth["table"] = nullptr;

        if (req.has_header("Authorization"))
        {
            const auto token = get_bearer_token_auth(req);
            auth["token"] = trim(token);
            auth["type"] = "user";
        }

        // Update the context
        ctx.set("auth", auth);
        return REQUEST_PENDING;
    }

    bool TableUnit::hasAccess(const Request& req, Response& res, Context& ctx)
    {
        TRACE_CLASS_METHOD();

        // Get the auth var from the context, resort to empty object if it's not set.
        auto auth = *ctx.get<json>("auth").value_or(new json{json::object()});

        // Store rule, depending on the request type
        std::string rule;
        if (req.method == "GET")
        {
            // Check if we are fetching single record using the ID
            // empty params means we are listing through the endpoint
            if (req.path_params.empty()) rule = m_listRule;
            else rule = m_getRule;
        }
        else if (req.method == "POST") rule = m_addRule;
        else if (req.method == "PATCH") rule = m_updateRule;
        else if (req.method == "DELETE") rule = m_deleteRule;
        else
        {
            json response;
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Unsupported method";

            res.status = 400;
            res.set_content(response.dump(), "application/json");
            return REQUEST_HANDLED;
        }

        // Remove whitespaces
        rule = trim(rule);

        // Token map variables for evaluation
        TokenMap vars;

        // Expand logged user if token is present and query user information if it exists
        if (auth.contains("token") && !auth["token"].is_null() && !auth["token"].empty())
        {
            const auto token = auth.value("token", "");

            // If token validation worked, lets get data from database
            if (const auto resp = JWT::verifyJWTToken(token, MantisApp::jwtSecretKey());
                resp.value("verified", false))
            {
                // Ensure that, we only enter this block if `id` and `table` keys have a valid string data
                if ((!resp["id"].is_null() && !resp["id"].empty())
                    && (!resp["table"].is_null() && !resp["table"].empty()))
                {
                    const auto _id = resp["id"].is_null() ? "" : resp.value("id", "");
                    const auto _table = resp["table"].is_null() ? "" : resp.value("table", "");

                    // Query for user with given ID, this info will be populated to the
                    // expression evaluator args as well as available through
                    // the session context, queried by:
                    //  ` ctx.get<json>("auth").value("id", ""); // returns the user ID
                    //  ` ctx.get<json>("auth").value("name", ""); // returns the user's name
                    auto sql = MantisApp::instance().db().session();
                    soci::row r;
                    std::string query = "SELECT * FROM " + _table + " WHERE id = :id LIMIT 1";
                    *sql << query, soci::use(_id), soci::into(r);

                    // Let's only populate TokenVars for now, no returning
                    if (sql->got_data())
                    {
                        // Populate the auth object with additional data from the database
                        // remove `password` field if available
                        auto user = _table == "__admins"
                                        ? parseDbRowToJson(r, MantisApp::instance().router().adminTableFields)
                                        : parseDbRowToJson(r);

                        // Populate the `auth` object
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

                        // Add `auth` data to the TokenMap
                        vars["auth"] = MantisApp::instance().evaluator().jsonToTokenMap(auth);
                    }
                }
            }
        }

        // Request Token Map
        TokenMap reqMap;
        reqMap["ip"] = req.remote_addr;
        reqMap["port"] = req.remote_port;

        try
        {
            if (req.method == "POST" && !req.body.empty())
            {
                // Parse request body and add it to the request TokenMap
                auto request = json::parse(req.body);
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
            if (auth.contains("table") && !auth["table"].is_null()
                && auth.value("table", "") == "__admins") // Check table value not null
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

            res.status = 403;
            res.set_content(response.dump(), "application/json");
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

        res.status = 403;
        res.set_content(response.dump(), "application/json");
        return REQUEST_HANDLED;
    }
}
