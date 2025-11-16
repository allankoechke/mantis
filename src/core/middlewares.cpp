//
// Created by codeart on 12/11/2025.
//

#include "../include/mantis/core/middlewares.h"
#include "../include/mantis/mantis.h"
#include "mantis/core/models/entity.h"

namespace mantis {
    std::function<bool(MantisRequest&, MantisResponse&)> getAuthToken()
    {
        return [](MantisRequest& req, MantisResponse& _) {
            // If we have an auth header, extract it into the ctx, else
            // add a guest user type. The auth if present, should have
            // the user id, auth table, etc.
            json auth;
            auth["type"] = "guest"; // or 'user' or 'admin'
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
        };
    }

    std::function<bool(MantisRequest&, MantisResponse&)> hasAccess()
    {
        return [](MantisRequest& req, MantisResponse& res) {
            // Get the auth var from the context, resort to empty object if it's not set.
            auto auth = req.getOr<json>("auth", json::object());

            logger::trace("Auth Obj: `{}`", auth.dump());

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
            std::string rule = "";

            // try {
            //     auto table = MantisBase::instance().entity(auth["table"].get<std::string>());
            //
            // rule = method == "GET"
            //                        ? (req.hasPathParams() ? table.listRule() : table.getRule())
            //                        : method == "POST"
            //                        ? table.addRule()
            //                        : method == "PATCH"
            //                        ? table.updateRule()
            //                        : table.deleteRule();
            // }

            logger::trace("Rule: `{}`", rule);

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
                    logger::trace("Token Verified: `{}`", token);
                    const auto user_id = resp.at("id").get<std::string>();
                    const auto user_table = resp.at("table").get<std::string>();

                    // Query for user with given ID, this info will be populated to the
                    // expression evaluator args as well as available through
                    // the session context, queried by:
                    //  ` req.get<json>("auth").value("id", ""); // returns the user ID
                    //  ` req.get<json>("auth").value("name", ""); // returns the user's name
                    auto sql = MantisBase::instance().db().session();
                    std::string query = "SELECT * FROM " + user_table + " WHERE id = :id LIMIT 1";

                    soci::row user_row;
                    *sql << query, soci::use(user_id), soci::into(user_row);

                    // Let's only populate TokenVars for now, no returning
                    if (sql->got_data())
                    {
                        // Populate the auth object with additional data from the database
                        // remove `password` field if available
                        auto user = user_table == "__admins"
                                        ? sociRow2Json(user_row, {}) // MantisBase::instance().router().adminTableFields)
                                        : sociRow2Json(user_row, {}); // TODO ... get table ...

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
                        vars["auth"] = MantisBase::instance().evaluator().jsonToTokenMap(auth);
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
                    reqMap["body"] = MantisBase::instance().evaluator().jsonToTokenMap(request);
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

                logger::trace("Table: `{}`", auth.at("table").get<std::string>());

                // User was not an admin, lets return access denied error
                json response;
                response["status"] = 403;
                response["data"] = json::object();
                response["error"] = "Admin auth required to access this resource.";

                res.sendJson(403, response);
                return REQUEST_HANDLED;
            }

            logger::trace("Expression Rule = {}", rule);

            // If expression evaluation returns true, lets return allowing execution
            // continuation. Else, we'll craft an error response.
            if (MantisBase::instance().evaluator().evaluate(rule, vars))
                return REQUEST_PENDING; // Proceed to next middleware

            // Evaluation yielded false, return generic access denied error
            json response;
            response["status"] = 403;
            response["data"] = json::object();
            response["error"] = "Access denied!";

            res.sendJson(403, response);
            return REQUEST_HANDLED;
        };
    }


    std::function<bool(MantisRequest&, MantisResponse&)> requireExprEval(const std::string& expr) {
        return [expr](MantisRequest &req, MantisResponse &res) {
            return REQUEST_PENDING;
        };
    }

    std::function<bool(MantisRequest &, MantisResponse &)> requireGuestOnly() {
        return [](MantisRequest &req, MantisResponse &res) {
            return REQUEST_PENDING;
        };
    }

    std::function<bool(MantisRequest &, MantisResponse &)> requireAdminAuth() {
        return [](MantisRequest &req, MantisResponse &res) {
            return REQUEST_PENDING;
        };
    }

    std::function<bool(MantisRequest &, MantisResponse &)> requireAdminOrEntityAuth(const std::string &entity_name) {
        return [entity_name](MantisRequest &req, MantisResponse &res) {
            return REQUEST_PENDING;
        };
    }

    std::function<bool(MantisRequest &, MantisResponse &)> requireEntityAuth(const std::string &entity_name) {
        return [entity_name](MantisRequest &req, MantisResponse &res) {
            return REQUEST_PENDING;
        };
    }
}
