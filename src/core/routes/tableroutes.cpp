//
// Created by allan on 20/05/2025.
//

#include "../../../include/mantis/core/routes/tableroutes.h"

#include "../../../include/mantis/utils.h"
#include "../../../include/mantis/app/app.h"
#include "../../../include/mantis/core/crud/tablescrud.h"

namespace mantis
{
    TableRoutes::TableRoutes(MantisApp* app,
                             const std::string& tableName,
                             const std::string& tableId,
                             const std::string& tableType)
        : TableUnit(app, tableName, tableId, tableType),
          m_crud(new TablesCrud(app))
    {
    }

    bool TableRoutes::setupRoutes()
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

    void TableRoutes::fetchRecord(const Request& req, Response& res, Context& ctx)
    {
        auto id = req.path_params.at("id");
        json response;
        if (id.empty())
        {
            response["status"] = 400;
            response["error"] = "Record ID is required";
            response["data"] = json{};

            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }

        try
        {
            const auto resp = m_crud->read(id, json{});
            if (resp.has_value())
            {
                response["status"] = 200;
                response["error"] = "";
                response["data"] = resp.value();

                res.set_content(response.dump(), "application/json");
                res.status = 200;
                return;
            }
            else
            {
                response["status"] = 404;
                response["error"] = "Item Not Found";
                response["data"] = json{};

                res.set_content(response.dump(), "application/json");
                res.status = 404;
                return;
            }
        }
        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["error"] = e.what();
            response["data"] = json{};

            res.set_content(response.dump(), "application/json");
            res.status = 500;
            return;
        }

        catch (...)
        {
            response["status"] = 500;
            response["error"] = "Unknown Error";
            response["data"] = json{};

            res.set_content(response.dump(), "application/json");
            res.status = 500;
            return;
        }
    }

    void TableRoutes::fetchRecords(const Request& req, Response& res, Context& ctx)
    {
        Log::info("Fetch all tables from database");
        json response;
        try
        {
            auto list = m_crud->list(json());
            response["data"] = list;
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

    void TableRoutes::createRecord(const Request& req, Response& res, Context& ctx)
    {
        json body, response;
        try
        {
            body = json::parse(req.body);
        }
        catch (const std::exception& e)
        {
            response["status"] = 400;
            response["error"] = e.what();
            response["data"] = json{};

            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }

        Log::debug("Create table, data = {}", body.dump());

        auto validateRequestBody = [&]() -> bool
        {
            if (trim(body.at("name").get<std::string>()).empty())
            {
                response["status"] = 400;
                response["error"] = "Table name is required";
                response["data"] = json{};

                res.status = 400;
                res.set_content(response.dump(), "application/json");
                return false;
            }

            // Check that the type is either a view|base|auth type
            auto type = body.at("type").get<std::string>();
            toLowerCase(type);
            if ( !(type == "view" || type == "base" || type == "auth") )
            {
                response["status"] = 400;
                response["error"] = "Table type should be either 'base', 'view', or 'auth'";
                response["data"] = json{};

                res.status = 400;
                res.set_content(response.dump(), "application/json");
                return false;
            }

            // If the table type is of view type, check that the SQL is passed in ...
            if (type == "view")
            {
                auto sql_stmt = body.at("sql").get<std::string>();
                trim(sql_stmt);
                if (sql_stmt.empty())
                {
                    response["status"] = 400;
                    response["error"] = "'view' table require an SQL Statement.";
                    response["data"] = json{};

                    res.status = 400;
                    res.set_content(response.dump(), "application/json");
                    return false;
                }
            }
            else
            {
                // Check fields if any is added
                for (const auto& field: body.at("fields").get<std::vector<json>>())
                {
                    // On the minimum, we need field name and type
                    auto field_name = field.at("name").get<std::string>();
                    auto field_type = field.at("type").get<std::string>();

                    if (trim(field_name).empty())
                    {
                        response["status"] = 400;
                        response["error"] = "One of the fields is missing a valid name";
                        response["data"] = json{};

                        res.status = 400;
                        res.set_content(response.dump(), "application/json");
                        return false;
                    }

                    if (trim(field_type).empty())
                    {
                        response["status"] = 400;
                        response["error"] = "Field type '" + field_type + "' for '" + field_name +"' is not a valid type";
                        response["data"] = json{};

                        res.status = 400;
                        res.set_content(response.dump(), "application/json");
                        return false;
                    }
                }
            }

            return true;
        };

        // Validate JSON body
        if (!validateRequestBody()) return;

        auto resp = m_crud->create(body, json{});
        if (!resp.value("error", json{}).empty())
        {
            int s = resp.at("error").at("status").get<int>();
            Log::debug("Table creation failed: {}", resp.at("error").dump());
            response["status"] = s > 0 ? s : 500;
            response["error"] = resp.at("error").at("error").get<std::string>();
            response["data"] = json{};

            res.set_content(response.dump(), "application/json");
            res.status = s > 0 ? s : 500;

            Log::critical("Failed to create table, reason: {}", resp.dump());
            return;
        }

        Log::debug("Table creation successful");

        response["status"] = 201;
        response["error"] = "";
        response["data"] = resp.at("data");

        res.set_content(response.dump(), "application/json");
        res.status = 201;
    }

    void TableRoutes::updateRecord(const Request& req, Response& res, Context& ctx)
    {
    }

    void TableRoutes::deleteRecord(const Request& req, Response& res, Context& ctx)
    {
        const auto id = req.path_params.at("id");
        json response;
        if (id.empty())
        {
            response["status"] = 400;
            response["error"] = "Record ID is required";
            response["data"] = json{};

            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }

        try
        {
            // If remove returns false, something didn't go right!
            if (const auto resp = m_crud->remove(id, json{}); !resp)
            {
                response["status"] = 500;
                response["error"] = "Could not delete record";
                response["data"] = json{};

                res.set_content(response.dump(), "application/json");
                res.status = 500;
                return;
            }
        } catch (const std::exception& e)
        {
            response["status"] = 500;
            response["error"] = e.what();
            response["data"] = json{};

            res.set_content(response.dump(), "application/json");
            res.status = 500;
            return;
        }

        Log::debug("Table deletion successful");

        res.set_content("", "application/json");
        res.status = 204;
    }
} // mantis
