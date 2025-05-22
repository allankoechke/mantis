//
// Created by allan on 20/05/2025.
//

#include "../../../include/mantis/core/routes/tableroutes.h"
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
    }

    void TableRoutes::fetchRecords(const Request& req, Response& res, Context& ctx)
    {
        Log::info("Fetch all tables from database");
        json response;
        try
        {
            json::array_t list = m_crud->list(json());
            response["data"] = list;
            response["status"] = 200;
            response["error"] = "";
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e)
        {
            response["data"] = json::array();
            response["status"] = 500;
            response["error"] = e.what();
            res.set_content(response.dump(), "application/json");
        }

        catch (...)
        {
            response["data"] = json::array();
            response["status"] = 500;
            response["error"] = "Internal Server Error";
            res.set_content(response.dump(), "application/json");
        }

        return;
    }

    void TableRoutes::createRecord(const Request& req, Response& res, Context& ctx)
    {
    }

    void TableRoutes::updateRecord(const Request& req, Response& res, Context& ctx)
    {
    }

    void TableRoutes::deleteRecord(const Request& req, Response& res, Context& ctx)
    {
    }
} // mantis
