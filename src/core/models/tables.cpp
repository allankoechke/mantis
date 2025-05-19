//
// Created by allan on 13/05/2025.
//

#include "../../../include/mantis/core/models/tables.h"
#include "../../../include/mantis/core/database.h"


mantis::TableUnit::TableUnit(
    MantisApp* app,
    const std::string& tableName,
    const std::string& tableId,
    const std::string& tableType)
    : m_app(app),
      m_tableName(tableName),
      m_tableId(tableId),
      m_tableType(tableType),
      m_listRule(Rule{}),
      m_getRule(Rule{}), m_addRule(Rule{}),
      m_updateRule(Rule{}), m_deleteRule(Rule{})
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

json mantis::TableUnit::fields() const
{
    return m_fields;
}

void mantis::TableUnit::setFields(const json& fields)
{
    m_fields = fields;
}

void mantis::TableUnit::setIsSystemTable(const bool isSystemTable)
{
    m_isSystem = isSystemTable;
}

mantis::Rule mantis::TableUnit::listRule()
{
    return m_listRule;
}

void mantis::TableUnit::setListRule(const Rule& rule)
{
    m_listRule = rule;
}

mantis::Rule mantis::TableUnit::getRule()
{
    return m_getRule;
}

void mantis::TableUnit::setGetRule(const Rule& rule)
{
    m_getRule = rule;
}

mantis::Rule mantis::TableUnit::addRule()
{
    return m_addRule;
}

void mantis::TableUnit::addRule(const Rule& rule)
{
    m_addRule = rule;
}

mantis::Rule mantis::TableUnit::updateRule()
{
    return m_updateRule;
}

void mantis::TableUnit::updateRule(const Rule& rule)
{
    m_updateRule = rule;
}

mantis::Rule mantis::TableUnit::deleteRule()
{
    return m_deleteRule;
}

void mantis::TableUnit::deleteRule(const Rule& rule)
{
    m_deleteRule = rule;
}

json& mantis::TableUnit::create(const json& entity, const json& opts)
{
    auto sql = m_app->db().session();
    soci::transaction tr(*sql);

    std::string columns, placeholders;
    // Build column list and placeholders from data
    // ...



    // Create a values object
    soci::values data;

    // Populate it in a loop
    for (const auto& field : m_fields) {
        data.set(field["name"], entity["name"]);
    }


    for (int i=0; i<m_fields.size(); ++i)
    {
        const auto& field = m_fields.at(i);
        std::string name = field["name"];

        columns += name;
        placeholders += ":" + name;

        if ( i < m_fields.size()-1)
        {
            columns += ",";
            placeholders += ",";
        }
    }

    std::string query = "INSERT INTO " + m_tableName + " (" + columns + ") VALUES (" + placeholders + ")";
    *sql << query, use(data);
}

std::optional<json> mantis::TableUnit::read(int id, const json& opts)
{
}

json& mantis::TableUnit::update(int id, const json& entity, const json& opts)
{
}

bool mantis::TableUnit::remove(int id, const json& opts)
{
}

std::vector<json> mantis::TableUnit::list(const json& opts)
{
}
