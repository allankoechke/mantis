#include "../../include/mantis/core/tables/tables.h"
#include "../../../include/mantis/mantisbase.h"
#include "../../include/mantis/core/database_mgr.h"
#include "../../include/mantis/utils/utils.h"
#include "../../include/mantis/core/jwt_mgr.h"
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

            // Verify user password
            if (const auto p_verified = verifyPassword(password, db_password); !p_verified)
            {
                response["status"] = 404;
                response["data"] = json::object();
                response["error"] = "No user found matching given email/password combination.";

                res.sendJson(404, response);
                logger::warn("No user found for given email/password combination");
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
            logger::critical("Error Processing Request: {}", e.what());
        }
        catch (...)
        {
            response["status"] = 500;
            response["data"] = json::object();
            response["error"] = "Internal Server Error";

            res.sendJson(500, response);
            logger::critical("Internal Server Error");
        }
    }

    void TableUnit::resetPassword(MantisRequest& req, MantisResponse& res)
    {
        TRACE_CLASS_METHOD();

        [[maybe_unused]] auto sql = MantisBase::instance().db().session();

        logger::trace("Resetting password on record, endpoint {}", req.getPath());
        //res.sendJson(200, req);
    }

}
