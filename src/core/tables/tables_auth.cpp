//
// Created by allan on 07/06/2025.
//

#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../include/mantis/utils.h"

namespace mantis
{
    void TableUnit::authWithEmailAndPassword(const Request& req, Response& res, Context& ctx)
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
            const auto sql = m_app->db().session();

            soci::row r;
            const auto query = "SELECT * FROM " + m_tableName + " WHERE email = :email LIMIT 1;";
            *sql << query, soci::use(email), soci::into(r);
            Log::trace("Executed Query: {}", query);

            if (!sql->got_data())
            {
                response["status"] = 404;
                response["data"] = json::object();
                response["error"] = "No user found matching given email/password combination.";

                res.status = 404;
                res.set_content(response.dump(), "application/json");

                Log::debug("No user found matching given email/password combination.");
                return;
            }

            // Extract user password value
            const auto db_password = r.get<std::string>("password");
            auto tokens = splitString(db_password, ":");
            if (tokens.size() < 2) // Check that splitting yield two parts ...
            {
                response["status"] = 404;
                response["data"] = json::object();
                response["error"] = "No user found matching given email/password combination.";

                res.status = 404;
                res.set_content(response.dump(), "application/json");

                Log::critical("Could not split database password");
                return;
            }

            if (tokens[0] == std::to_string(std::hash<std::string>{}(password + tokens[1])))
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
                Log::info("Login Successful, user: {}", response.dump());
                return;
            }

            response["status"] = 404;
            response["data"] = json::object();
            response["error"] = "No user found matching given email/password combination.";

            res.status = 404;
            res.set_content(response.dump(), "application/json");

            Log::debug("No user found for given email/password combination.");
            return;
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
        [[maybe_unused]] auto sql = m_app->db().session();

        Log::trace("Resetting password on record, endpoint {}", req.path);
        res.status = 200;
        res.set_content(req.body, "application/json");
    }

    bool TableUnit::getAuthToken(const Request& req, [[maybe_unused]] Response& res, Context& ctx)
    {
        Log::trace("Extracting Auth Token on path {}", req.path);

        // If we have an auth header, extract it into the ctx, else
        // add a guest user type. The auth if present, should have
        // the user id, auth table, etc.
        json auth;
        auth["token"] = "";
        auth["type"] = "guest"; // or 'user'
        auth["table"] = "";

        if (req.has_header("Authorization"))
        {
            const auto token = get_bearer_token_auth(req);
            auth["token"] = trim(token);
            auth["type"] = "user";
            auth["id"] = "";
            auth["table"] = "";
        }

        // Update the context
        ctx.set("auth", auth);
        return REQUEST_PENDING;
    }

    bool TableUnit::hasAccess(const Request& req, Response& res, Context& ctx)
    {
        // Get the auth var from the context, resort to empty object if it's not set.
        auto auth = ctx.get<json>("auth").has_value() ? *ctx.get<json>("auth").value() : json::object();

        // Store rule, depending on the request type
        std::string rule;
        if (req.method == "GET")
        {
            // Check if we are fetching single record using the ID
            // empty params means we are listing through the endpoint
            if (req.path_params.empty()) rule = m_listRule;
            else rule = m_getRule;
        }
        else if (req.method == "POST") { rule = m_addRule; }
        else if (req.method == "PATCH") { rule = m_addRule; }
        else if (req.method == "DELETE") { rule = m_addRule; }
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

        // Expand logged user if token is present
        // Maybe Query User information if it exists?
        // id, tableName, ...
        if (const auto& token = auth.value("token", ""); !token.empty())
        {
            const auto resp = JWT::verifyJWTToken(token, MantisApp::jwtSecretKey());
            if (!resp.value("verified", false) || !resp.value("error", "").empty())
            {
                json response;
                response["status"] = 403;
                response["data"] = json::object();
                response["error"] = resp.value("error", "");

                res.status = 403;
                res.set_content(response.dump(), "application/json");
                return REQUEST_HANDLED;
            }

            // Extract and verify that the id and table data is provided, else,
            // return an error
            const auto _id = resp.value("id", "");
            const auto _table = resp.value("table", "");

            if (_id.empty() || _table.empty())
            {
                json response;
                response["status"] = 403;
                response["data"] = json::object();
                response["error"] = "Auth token missing user id or table name";

                res.status = 403;
                res.set_content(resp.dump(), "application/json");
                return REQUEST_HANDLED;
            }

            // Query for user with given ID, this info will be populated to the
            // expression evaluator args as well as available through
            // the session context, queried by:
            //  ` ctx.get<json>("auth").value("id", ""); // returns the user ID
            //  ` ctx.get<json>("auth").value("name", ""); // returns the user's name
            auto sql = m_app->db().session();
            soci::row r;
            std::string query = "SELECT * FROM " + _table + " WHERE id = :id LIMIT 1";
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
        }

        // Token map variables
        cparse::TokenMap vars;

        auto request = json::object();
        try { request = json::parse(req.body); }
        catch (...)
        {
        }

        // Add `auth` and `request` details to the `TokenMap` var stack for use in
        // expression evaluation. First, convert the `json` to `TokenMap`.
        vars["auth"] = m_app->evaluator().jsonToTokenMap(auth);
        vars["req"] = m_app->evaluator().jsonToTokenMap(request);

        // If the rule is empty, enforce admin login
        if (rule.empty())
        {
            const auto table_name = auth.value("table", "");
            // Check if user is logged in as Admin
            if (table_name == "__admin")
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
        if (m_app->evaluator().evaluate(rule, vars))
            return REQUEST_PENDING; // Proceed to next mware

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
