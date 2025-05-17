//
// Created by allan on 13/05/2025.
//

#include "../../../include/mantis/core/models/tables.h"


mantis::TableUnit::TableUnit(
    MantisApp* app,
    const std::string& tableName,
    const std::string& tableId,
    const std::string& tableType,
    Rule listRule,
    Rule getRule,
    Rule addRule,
    Rule updateRule,
    Rule deleteRule)
    : m_app(app), m_tableName(tableName),
      m_tableId(tableId), m_tableType(tableType), m_listRule(listRule),
      m_getRule(getRule), m_addRule(addRule),
      m_updateRule(updateRule), m_deleteRule(deleteRule)
{
}

void mantis::TableUnit::setRouteDisplayName(const std::string& routeName)
{
    if (routeName.empty())
        return;

    m_routeName = routeName;
}

bool mantis::TableUnit::setupRoutes()
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

        // Add/Update and Delete are not supported in views
        if (m_tableType != "view")
        {
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


void mantis::TableUnit::fetchRecord(const Request& req, Response& res, Context& ctx)
{
    Log::debug("TableMgr::FetchRecord for {}", req.path);
    ctx.dump();

    json response;
    response["status"] = 200;
    response["body"] = json{};
    response["message"] = "";

    res.status = 200;
    res.set_content(response.dump(), "application/json");
}

void mantis::TableUnit::fetchRecords(const Request& req, Response& res, Context& ctx)
{
    Log::debug("TableMgr::FetchRecords for {}", req.path);
    ctx.dump();

    json response;
    response["status"] = 200;
    response["body"] = json::array();
    response["message"] = "";

    //  Add pagination

    res.status = 200;
    res.set_content(response.dump(), "application/json");
}

void mantis::TableUnit::createRecord(const Request& req, Response& res, Context& ctx)
{
    Log::debug("TableMgr::CreateRecord for {}", req.path);
    ctx.dump();

    json response;
    response["status"] = 201;
    response["body"] = json::array();
    response["message"] = "Record created";

    res.status = 201;
    res.set_content(response.dump(), "application/json");
}

void mantis::TableUnit::updateRecord(const Request& req, Response& res, Context& ctx)
{
    Log::debug("TableMgr::UpdateRecord for {}", req.path);
    ctx.dump();

    json response;
    response["status"] = 200;
    response["body"] = json{};
    response["message"] = "Record Updated";

    res.status = 200;
    res.set_content(response.dump(), "application/json");
}

void mantis::TableUnit::deleteRecord(const Request& req, Response& res, Context& ctx)
{
    Log::debug("TableMgr::DeleteRecord for {}", req.path);
    ctx.dump();

    json response;
    response["status"] = 204;
    response["body"] = json{};
    response["message"] = "Record Deleted";

    res.status = 204;
    res.set_content(response.dump(), "application/json");
}

bool mantis::TableUnit::authWithPassword(const std::string& email, std::string& password)
{
    return true;
}

bool mantis::TableUnit::getAuthToken(const Request& req, [[maybe_unused]] Response& res, Context& ctx)
{
    Log::debug("TableMgr::HasAuthHeader for {}", req.path);

    // If we have an auth header, extract it into the ctx, else
    // add a guest user type. The auth if present, should have
    // the user id, auth table, etc.
    json auth;
    auth["token"] = "";
    auth["type"] = "guest"; // or 'user'
    auth["user"] = json{};

    if (req.has_header("Authorization"))
    {
        auth["token"] = get_bearer_token_auth(req);
        auth["type"] = "user";

        // Maybe Query User information if it exists?
        // id, tableName, ...
    }

    // Update the context
    ctx.set("auth", auth);
    ctx.dump();

    return true;
}

bool mantis::TableUnit::hasAccess(const Request& req, Response& res, Context& ctx)
{
    Log::debug("TableMgr::HasAccessPermission for {}", req.path);
    ctx.dump();
    return true;
}

std::string mantis::TableUnit::tableName()
{
    return m_tableName;
}

std::string mantis::TableUnit::tableId()
{
    return m_tableId;
}

std::string mantis::TableUnit::tableType()
{
    return m_tableType;
}

mantis::Rule mantis::TableUnit::listRule()
{
    return m_listRule;
}

mantis::Rule mantis::TableUnit::getRule()
{
    return m_getRule;
}

mantis::Rule mantis::TableUnit::addRule()
{
    return m_addRule;
}

mantis::Rule mantis::TableUnit::updateRule()
{
    return m_updateRule;
}

mantis::Rule mantis::TableUnit::deleteRule()
{
    return m_deleteRule;
}
