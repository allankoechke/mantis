//
// Created by allan on 08/05/2025.
//

#include <memory>
#include <mantis/core/database.h>
#include <mantis/core/models.h>
#include <mantis/mantis.h>
#include <mantis/utils.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include "../3rdParty/soci/src/backends/sqlite3/common.h"

Mantis::DatabaseMgr::DatabaseMgr(const MantisApp& app)
    : m_app(make_shared<MantisApp>(app)),
    m_poolSize(2),
    m_databaseType(SQLITE) {}

bool Mantis::DatabaseMgr::DbInit()
{
    // Connection string required for all other databases
    if (!m_databaseType == SQLITE) return false;

    return DbInit(SQLITE, std::string());
}

bool Mantis::DatabaseMgr::DbInit(const std::string& connectionString)
{
    return DbInit(m_databaseType, connectionString);
}

bool Mantis::DatabaseMgr::DbInit(const DatabaseType& dbType, const std::string& connectionString)
{
    // If pool size is invalid, just return
    if (m_poolSize <= 0) return false;

    // All databases apart from SQLite should pass in a connection string
    if (dbType != SQLITE && connectionString.empty()) return false;

    // Update database type
    m_databaseType = dbType;

    try
    {
        // Create connection pool instance
        m_dbPool = std::make_unique<soci::connection_pool>(m_poolSize);

        // Populate the pools with db connections
        for (std::size_t i = 0; i < m_poolSize; ++i)
        {
            switch(dbType)
            {
            case SQLITE:
                {
                    // For SQLite, lets explicitly define location and name of the database
                    // we intend to use within the `dataDir`
                    auto conn_str = "db=" + JoinPaths(m_app->DataDir(), "vault.db").string();
                    soci::session& sql = m_dbPool->at(i);
                    sql.open(soci::sqlite3, conn_str);
                    break;
                }

                // For other DB types
                // Connect to the database URL, no checks here,
                // make your damn checks to ensure the URL is VALID!
                // TODO maybe add checks?

            default:
                Logger::Warn("Database Connection for '{}' Not Implemented Yet!", connectionString);
            }
        }

        return true;
    }

    catch (const soci::soci_error& e)
    {
        Logger::Critical("Database Connection SOCI::Error: {}", e.what());
    }

    catch (const std::exception& e)
    {
        Logger::Critical("Database Connection std::exception: {}", e.what());
    }

    catch (...)
    {
        Logger::Critical("Database Connection Failed: Unknown Error");
    }

    return false;
}

std::shared_ptr<soci::session> Mantis::DatabaseMgr::DbSession() const
{
    // Create a shared session pointer and return it.
    auto sql = std::make_shared<soci::session>(*m_dbPool);
    return sql;
}

void Mantis::DatabaseMgr::SetPoolSize(const int& poolSize)
{
    m_poolSize = poolSize;
}

void Mantis::DatabaseMgr::SetDatabaseType(const DatabaseType& dbType)
{
    m_databaseType = dbType;
}

bool Mantis::DatabaseMgr::EnsureDatabaseSchemaLoaded() const
{
    if (!CreateSystemTables())
        m_app->Quit(-1, "Failed to create database schema");

    std::cout << GenerateSystemDatabaseSchema() << std::endl;
    return true;
}

bool Mantis::DatabaseMgr::CreateSystemTables() const
{
    // Create system tables as follows:
    // __tables for managing user tables & schema
    // __admins for managing system admin users
    // __logs * optional for logs
    // Any other?

    try
    {
        const auto db = DbSession();
        soci::transaction tr(*db);

        AdminTable admin;
        admin.name = "_admin_";
        Logger::Debug("Generated Admin Table SQL: '{}'", admin.to_sql());
        *db << admin.to_sql();


        return true;

        // '__admin' table
        stringstream ss;
        ss << "CREATE TABLE IF NOT EXISTS __admin";
        ss << " (id TEXT PRIMARY KEY, email TEXT, password TEXT,";
        ss << " created TEXT, updated TEXT); ";

        *db << ss.str();

        // '__tables' table
        ss.clear();
        ss << "CREATE TABLE IF NOT EXISTS __tables (";
        ss << " id TEXT PRIMARY KEY,";
        ss << " name TEXT NOT NULL,";
        ss << " type TEXT NOT NULL CHECK(type IN ('base', 'auth', 'view')),";
        ss << " schema TEXT NOT NULL );";

        *db << ss.str();

        // Commit transaction
        tr.commit();

        return true;
    }
    catch (std::exception& e)
    {
        Logger::Critical("Create System Tables Failed: {}", e.what());
    }

    return false;
}

std::string Mantis::DatabaseMgr::GenerateSystemDatabaseSchema() const
{
    Mantis::BaseTable sysTableSchema;
    sysTableSchema.name = "__tables";
    sysTableSchema.enableSync = true;
    sysTableSchema.id = "";
    sysTableSchema.type = TableType::Base;

    return sysTableSchema.to_sql();
}
