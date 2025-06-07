//
// Created by allan on 07/06/2025.
//

#include "../../include/mantis/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../include/mantis/utils.h"

namespace mantis
{
    void TableUnit::authWithEmailAndPassword(const Request& req, Response& res, Context& ctx) const
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
        auth["user"] = json::object();

        if (req.has_header("Authorization"))
        {
            const auto token = get_bearer_token_auth(req);
            auth["token"] = token;
            auth["type"] = "user";
            auth["id"] = "";
            auth["table"] = "";

            // Maybe Query User information if it exists?
            // id, tableName, ...
            const auto resp = JWT::verifyJWTToken(token, MantisApp::jwtSecretKey());
            if (!resp.value("verified", false) || !resp.value("error", "").empty())
            {
                json response;
                response["status"] = 403;
                response["data"] = json::object();
                response["error"] = resp.value("error", "");

                res.status = 403;
                res.set_content(response.dump(), "application/json");
                return false;
            }

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
                return false;
            }

            auto sql = m_app->db().session();
            soci::row r;
            std::string query = "SELECT * FROM " + _table + " WHERE id = :id LIMIT 1";
            *sql << query, soci::use(_id), soci::into(r);

            if (!sql->got_data())
            {
                json response;
                response["status"] = 403;
                response["data"] = json::object();
                response["error"] = "Auth id was not found.";

                res.status = 403;
                res.set_content(resp.dump(), "application/json");
                return false;
            }

            auto user = parseDbRowToJson(r);
            auth["type"] = "user";
            auth["user"] = user;
            auth["id"] = _id;
            auth["table"] = _table;
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
}
