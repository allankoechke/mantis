//
// Created by allan on 07/06/2025.
//

#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../../include/mantis/utils/utils.h"

namespace mantis
{
    bool TableUnit::setupRoutes()
    {
        if (m_tableName.empty() && m_routeName.empty()) return false;

        const auto path = m_routeName.empty() ? m_tableName : m_routeName;
        const auto basePath = "/api/v1/" + path;

        try
        {
            // Fetch All Records
            Log::debug("Adding GET Request for table '{}'", basePath);
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
            Log::debug("Adding GET/1 Request for table '{}'", basePath);
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

            // Add/Update and Delete are not supported in views
            if (m_tableType != "view")
            {
                // Add Record
                Log::debug("Adding POST Request for table '{}'", basePath);
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
                Log::debug("Adding PATCH Request for table '{}'", basePath);
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
                Log::debug("Adding DELETE Request for table '{}'", basePath);
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
            }

            // Add auth endpoints for login
            if (m_tableType == "auth")
            {
                // Add Record
                Log::debug("Adding POST Request for table '{}/auth-with-password'", basePath);
                 MantisApp::instance().http().Post(
                    basePath + "/auth-with-password",
                    [this](const Request& req, Response& res, Context& ctx) -> void
                    {
                        authWithEmailAndPassword(req, res, ctx);
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
        Log::trace("Fetching record from endpoint {}", req.path);

        // Extract request ID and check that it's not empty
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
            // For every read, check that the optional<T> has a value.
            // If it's not null, get the data and respond back to the client
            // else, handle the 404 NOT FOUND response to the client
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

        // For any server errors, send it back to the client
        // Capture std::exception, and a catch-all block as well
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
        Log::trace("Fetching all record from endpoint {}", req.path);

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
        Log::trace("Creating new record, endpoint {}", req.path);

        json body, response;
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

        // Validate JSON body, return any validation errors encountered
        if (const auto resp = validateRequestBody(body))
        {
            response["status"] = resp.value().value("status", 500);
            response["error"] = resp.value().value("error", "");
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = resp.value().value("status", 500);

            Log::critical("Error Validating Field: {}", resp.value().value("error", ""));
            return;
        };

        // Try creating the record, if it checkMinValueFuncs, return the error
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

        // Get the data, for auth types, redact the password information
        // it's not useful data to return in the response despite being hashed
        auto record = respObj.at("data");
        Log::trace("Record creation successful: {}", record.dump());

        // For auth types, remove the password field from the response
        if (m_tableType == "auth" && record.contains("password"))
        {
            record.erase("password");
        }

        // Return the added record + the system generated fields
        response["status"] = 201;
        response["error"] = "";
        response["data"] = record;

        res.status = 201;
        res.set_content(response.dump(), "application/json");
    }

    void TableUnit::updateRecord(const Request& req, Response& res, Context& ctx)
    {
        Log::trace("Updating record, endpoint {}", req.path);

        json body, response;
        // Extract request ID and check that it's not empty
        const auto id = req.path_params.at("id");

        // For empty IDs, return 400, BAD REQUEST
        if (id.empty())
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Record ID is required!";

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

        // Validate JSON body, return any validation errors encountered
        if (const auto resp = validateUpdateRequestBody(body))
        {
            response["status"] = resp.value().value("status", 500);
            response["error"] = resp.value().value("error", "");
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = resp.value().value("status", 500);

            Log::critical("Error Validating Field: {}", resp.value().value("error", ""));
            return;
        };

        // Try creating the record, if it checkMinValueFuncs, return the error
        auto respObj = update(id, body, json{});
        if (!respObj.value("error", "").empty())
        {
            int status = respObj.value("status", 500);
            response["status"] = status;
            response["error"] = respObj.at("error").get<std::string>();
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = status;

            Log::critical("Failed to update record, id = {}, reason: {}", id, respObj.dump());
            return;
        }

        // Get the data, for auth types, redact the password information
        // it's not useful data to return in the response despite being hashed
        auto record = respObj.at("data");
        Log::trace("Record update successful: {}", record.dump());

        // For auth types, remove the password field from the response
        if (m_tableType == "auth" && record.contains("password"))
        {
            record.erase("password");
        }

        // Return the added record + the system generated fields
        response["status"] = 200;
        response["error"] = "";
        response["data"] = record;

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    }

    void TableUnit::deleteRecord(const Request& req, Response& res, Context& ctx)
    {
        Log::trace("Deleting record, endpoint {}", req.path);

        // Extract request ID and check that it's not empty
        const auto id = req.path_params.at("id");

        // For empty IDs, return 400, BAD REQUEST
        if (id.empty())
        {
            json response;
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Record ID is required!";

            res.status = 400;
            res.set_content(response.dump(), "application/json");
            return;
        }

        // The delete record may throw an error, lets catch it here and return
        // appropriately.
        try
        {
            [[maybe_unused]] const auto resp = remove(id, json::object());

            // If all went well, lets return Status OK response
            // no need of a body here
            res.status = 204;
            res.set_content("", "application/json");
        }
        catch (const std::exception& e)
        {
            json response;
            response["status"] = 404;
            response["data"] = json::object();
            response["error"] = e.what();

            res.status = 404;
            res.set_content(response.dump(), "application/json");
        }
    }
}
