//
// Created by allan on 16/05/2025.
//

#include "../../include/mantis/core/database.h"
#include "../../include/mantis/core/logging.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/utils.h"
#include "../../include/mantis/core/models/models.h"

#include <soci/sqlite3/soci-sqlite3.h>

mantis::DatabaseUnit::DatabaseUnit(MantisApp* app)
    : m_app(app), m_connPool(nullptr) {}

bool mantis::DatabaseUnit::connect(const DbType backend, const std::string& conn_str) {
    // If pool size is invalid, just return
    if (m_app->poolSize() <= 0)
        throw "Session pool size must be greater than 0";

    // All databases apart from SQLite should pass in a connection string
    if (m_app->dbType() != DbType::SQLITE && conn_str.empty())
        throw "Connection string for database is required!";

    try
    {
        // Create connection pool instance
        m_connPool = std::make_unique<soci::connection_pool>(m_app->poolSize());

        // Populate the pools with db connections
        for (std::size_t i = 0; i < m_app->poolSize(); ++i)
        {
            switch(m_app->dbType())
            {
            case DbType::SQLITE:
                {
                    // For SQLite, lets explicitly define location and name of the database
                    // we intend to use within the `dataDir`
                    auto sqlite_str = "db=" + joinPaths(m_app->dataDir(), "mantis.db").string();
                    soci::session& sql = m_connPool->at(i);
                    sql.open(soci::sqlite3, sqlite_str);
                    break;
                }

                // For other DB types
                // Connect to the database URL, no checks here,
                // make your damn checks to ensure the URL is VALID!
                // TODO maybe add checks?

            default:
                Log::warn("Database Connection for '{}' Not Implemented Yet!", conn_str);
            }
        }
    }

    catch (const soci::soci_error& e)
    {
        Log::critical("Database Connection SOCI::Error: {}", e.what());
    }

    catch (const std::exception& e)
    {
        Log::critical("Database Connection std::exception: {}", e.what());
    }

    catch (...)
    {
        Log::critical("Database Connection Failed: Unknown Error");
    }

    return false;
}

void mantis::DatabaseUnit::migrate() {
    // *m_sql << "CREATE TABLE IF NOT EXISTS users ("
    //          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    //          "name TEXT NOT NULL,"
    //          "email TEXT NOT NULL UNIQUE);";

    // Create system tables as follows:
    // __tables for managing user tables & schema
    // __admins for managing system admin users
    // __logs * optional for logs
    // Any other?

    try
    {
        auto sql = session();
        soci::transaction tr{*sql};

        AdminTable admin(m_app);
        admin.name = "__admin";
        *sql << admin.to_sql();
        Log::debug("Generated Admin Table SQL:  {}", admin.to_sql());
        Log::debug("Generated Admin Table JSON: {}", admin.to_json().dump());

        SystemTable tables(m_app);
        tables.name = "__tables";
        tables.fields.push_back(Field("name", FieldType::STRING, true, false, true));
        tables.fields.push_back(Field("type", FieldType::STRING, true, false, true));
        tables.fields.push_back(Field("schema", FieldType::STRING, true, false, true));
        tables.fields.push_back(Field("has_api", FieldType::UINT8, true, false, true));
        *sql << tables.to_sql();

        Log::debug("Generated Sys Tables SQL:  {}", tables.to_sql());
        Log::debug("Generated Sys Tables JSON: {}", tables.to_json().dump());

        // Commit transaction
        tr.commit();
    }
    catch (std::exception& e)
    {
        Log::critical("Create System Tables Failed: {}", e.what());
    }

    // Create Base Table
    // mantis::BaseTable sysTableSchema;
    // sysTableSchema.name = "__tables";
    // sysTableSchema.enableSync = true;
    // sysTableSchema.id = "";
    // sysTableSchema.type = TableType::Base;
    //
    // sysTableSchema.to_sql();
}

std::shared_ptr<soci::session> mantis::DatabaseUnit::session() const {
    return std::make_shared<soci::session>(*m_connPool);
}

soci::connection_pool& mantis::DatabaseUnit::connectionPool() const
{
    return *m_connPool;
}

bool mantis::DatabaseUnit::isConnected() const
{
    if (m_connPool == nullptr) return false;

    const auto sql = session();
    return sql->is_connected();
}

// std::shared_ptr<mantis::DatabaseUnit::UserCrud> mantis::DatabaseUnit::users() {
//     return user_crud_;
// }
//
// mantis::DatabaseUnit::UserCrud::UserCrud(soci::session& sql) : m_sql(sql) {}
//
// bool mantis::DatabaseUnit::UserCrud::create(const User& entity) {
//     m_sql << "INSERT INTO users(name, email) VALUES(:name, :email)",
//         soci::use(entity.name), soci::use(entity.email);
//     return true;
// }
//
// std::optional<mantis::User> mantis::DatabaseUnit::UserCrud::read(int id) {
//     User user;
//     soci::indicator ind;
//     try {
//         m_sql << "SELECT id, name, email FROM users WHERE id = :id",
//             soci::use(id), soci::into(user.user_id, ind), soci::into(user.name), soci::into(user.email);
//         if (ind == soci::i_ok)
//             return user;
//     } catch (...) {}
//     return std::nullopt;
// }
//
// bool mantis::DatabaseUnit::UserCrud::update(int id, const User& entity) {
//     m_sql << "UPDATE users SET name = :name, email = :email WHERE id = :id",
//         soci::use(entity.name), soci::use(entity.email), soci::use(id);
//     return true;
// }
//
// bool mantis::DatabaseUnit::UserCrud::remove(int id) {
//     m_sql << "DELETE FROM users WHERE id = :id", soci::use(id);
//     return true;
// }
//
// std::vector<mantis::User> mantis::DatabaseUnit::UserCrud::list() {
//     soci::rowset<soci::row> rs = (m_sql.prepare << "SELECT id, name, email FROM users");
//     std::vector<User> users;
//     for (auto const& r : rs) {
//         User user;
//         user.user_id = r.get<int>(0);
//         user.name = r.get<std::string>(1);
//         user.email = r.get<std::string>(2);
//         users.push_back(user);
//     }
//     return users;
// }

