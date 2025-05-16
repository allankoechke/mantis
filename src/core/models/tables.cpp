//
// Created by allan on 13/05/2025.
//

#include "../../../include/mantis/core/models/tables.h"


mantis::TableMgr::TableMgr(
    const Router& svrMgr,
    const std::string& tableName,
    const std::string& tableId,
    const std::string& tableType,
    Rule listRule,
    Rule getRule,
    Rule addRule,
    Rule updateRule,
    Rule deleteRule)
    : m_svrMgr(make_shared<Router>(svrMgr)), m_tableName(tableName),
      m_tableId(tableId), m_tableType(tableType), m_listRule(listRule),
      m_getRule(getRule), m_addRule(addRule),
      m_updateRule(updateRule), m_deleteRule(deleteRule)
{
    m_svrMgr;
}

void mantis::TableMgr::SetRouteDisplayName(const std::string& routeName)
{
    if (routeName.empty())
        return;

    m_routeName = routeName;
}

bool mantis::TableMgr::SetupRoutes()
{
    const auto path = m_routeName.empty() ? m_tableName : m_routeName;
    const auto basePath = "/api/tables/" + path;

    try
    {
        // Fetch All Records
        Logger::Debug("Adding GET Request for table '{}'", m_tableName);
        m_svrMgr->GetHttpServer()->get(
            basePath,
            [this](const Request& req, Response& res, Context& ctx)-> void
            {
                FetchRecords(req, res, ctx);
            },
            {
                [this](const Request& req, Response& res, Context& ctx)-> bool
                {
                    return HasAuthHeader(req, res, ctx);
                },
                [this](const Request& req, Response& res, Context& ctx)-> bool
                {
                    return HasAccessPermission(req, res, ctx);
                }
            }
        );

        // Fetch Single Record
        Logger::Debug("Adding GET/1 Request for table '{}'", m_tableName);
        m_svrMgr->GetHttpServer()->get(
            basePath + "/:id",
            [this](const Request& req, Response& res, Context& ctx)-> void
            {
                FetchRecord(req, res, ctx);
            },
            {
                [this](const Request& req, Response& res, Context& ctx)-> bool
                {
                    return HasAuthHeader(req, res, ctx);
                },
                [this](const Request& req, Response& res, Context& ctx)-> bool
                {
                    return HasAccessPermission(req, res, ctx);
                }
            }
        );

        // Add/Update and Delete are not supported in views
        if (m_tableType != "view")
        {
            // Add Record
            Logger::Debug("Adding POST Request for table '{}'", m_tableName);
            m_svrMgr->GetHttpServer()->post(
                basePath, [this](const Request& req, Response& res, Context& ctx)-> void
                {
                    CreateRecord(req, res, ctx);
                },
                {
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return HasAuthHeader(req, res, ctx);
                    },
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return HasAccessPermission(req, res, ctx);
                    }
                }
            );

            // Update Record
            Logger::Debug("Adding PATCH Request for table '{}'", m_tableName);
            m_svrMgr->GetHttpServer()->patch(
                basePath + "/:id",
                [this](const Request& req, Response& res, Context& ctx)-> void
                {
                    UpdateRecord(req, res, ctx);
                },
                {
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return HasAuthHeader(req, res, ctx);
                    },
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return HasAccessPermission(req, res, ctx);
                    }
                }
            );

            // Delete Record
            Logger::Debug("Adding DELETE Request for table '{}'", m_tableName);
            m_svrMgr->GetHttpServer()->delete_(
                basePath + "/:id",
                [this](const Request& req, Response& res, Context& ctx)-> void
                {
                    DeleteRecord(req, res, ctx);
                },
                {
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return HasAuthHeader(req, res, ctx);
                    },
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return HasAccessPermission(req, res, ctx);
                    }
                }
            );
        }

        return true;
    }

    catch (const std::exception& e)
    {
        Logger::Critical("Failed to create routes for table '{}' of '{}' type: {}",
                         m_tableName, m_tableType, e.what());
        return false;
    }

    catch (...)
    {
        Logger::Critical("Failed to create routes for table '{}' of '{}' type: {}",
                         m_tableName, m_tableType, "Unknown Error!");
        return false;
    }
}


void mantis::TableMgr::FetchRecord(const Request& req, Response& res, Context& ctx)
{
    Logger::Debug("TableMgr::FetchRecord for {}", req.path);
    ctx.dump();

    json response;
    response["status"] = 200;
    response["body"] = json{};
    response["message"] = "";

    res.status = 200;
    res.set_content(response.dump(), "application/json");
}

void mantis::TableMgr::FetchRecords(const Request& req, Response& res, Context& ctx)
{
    Logger::Debug("TableMgr::FetchRecords for {}", req.path);
    ctx.dump();

    json response;
    response["status"] = 200;
    response["body"] = json::array();
    response["message"] = "";

    //  Add pagination

    res.status = 200;
    res.set_content(response.dump(), "application/json");
}

void mantis::TableMgr::CreateRecord(const Request& req, Response& res, Context& ctx)
{
    Logger::Debug("TableMgr::CreateRecord for {}", req.path);
    ctx.dump();

    json response;
    response["status"] = 201;
    response["body"] = json::array();
    response["message"] = "Record created";

    res.status = 201;
    res.set_content(response.dump(), "application/json");
}

void mantis::TableMgr::UpdateRecord(const Request& req, Response& res, Context& ctx)
{
    Logger::Debug("TableMgr::UpdateRecord for {}", req.path);
    ctx.dump();

    json response;
    response["status"] = 200;
    response["body"] = json{};
    response["message"] = "Record Updated";

    res.status = 200;
    res.set_content(response.dump(), "application/json");
}

void mantis::TableMgr::DeleteRecord(const Request& req, Response& res, Context& ctx)
{
    Logger::Debug("TableMgr::DeleteRecord for {}", req.path);
    ctx.dump();

    json response;
    response["status"] = 204;
    response["body"] = json{};
    response["message"] = "Record Deleted";

    res.status = 204;
    res.set_content(response.dump(), "application/json");
}

bool mantis::TableMgr::AuthWithPassword(const std::string& email, std::string& password)
{
    return true;
}

bool mantis::TableMgr::HasAuthHeader(const Request& req, [[maybe_unused]] Response& res, Context& ctx)
{
    Logger::Debug("TableMgr::HasAuthHeader for {}", req.path);

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

bool mantis::TableMgr::HasAccessPermission(const Request& req, Response& res, Context& ctx)
{
    Logger::Debug("TableMgr::HasAccessPermission for {}", req.path);
    ctx.dump();
    return true;
}

std::string mantis::TableMgr::tableName()
{
    return m_tableName;
}

std::string mantis::TableMgr::tableId()
{
    return m_tableId;
}

mantis::Rule mantis::TableMgr::listRule()
{
    return m_listRule;
}

mantis::Rule mantis::TableMgr::getRule()
{
    return m_getRule;
}

mantis::Rule mantis::TableMgr::addRule()
{
    return m_addRule;
}

mantis::Rule mantis::TableMgr::updateRule()
{
    return m_updateRule;
}

mantis::Rule mantis::TableMgr::deleteRule()
{
    return m_deleteRule;
}
