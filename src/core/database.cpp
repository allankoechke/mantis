//
// Created by allan on 16/05/2025.
//

#include "../../include/mantis/core/database.h"
#include "../../include/mantis/core/logging.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/utils/utils.h"
#include "../../include/mantis/core/models/models.h"
#include "../../include/mantis/core/settings.h"

#include <soci/sqlite3/soci-sqlite3.h>
#include <private/soci-mktime.h>

#if MANTIS_HAS_POSTGRESQL
#include <soci/postgresql/soci-postgresql.h>
// #include "soci/postgresql/soci-postgresql.h"
// soci::dynamic_backends::register_backend("postgresql", soci::postgresql);
#endif

#define __file__ "core/tables/sys_tables.cpp"

mantis::DatabaseUnit::DatabaseUnit() : m_connPool(nullptr)
{
}

mantis::DatabaseUnit::~DatabaseUnit()
{
    disconnect();
}

bool mantis::DatabaseUnit::connect([[maybe_unused]] const DbType backend, const std::string& conn_str)
{
    // If pool size is invalid, just return
    if (MantisApp::instance().poolSize() <= 0)
        throw std::runtime_error("Session pool size must be greater than 0");

    // All databases apart from SQLite should pass in a connection string
    if (MantisApp::instance().dbType() != DbType::SQLITE && conn_str.empty())
        throw std::runtime_error("Connection string for database is required!");

    try
    {
        // Create connection pool instance
        m_connPool = std::make_unique<soci::connection_pool>(MantisApp::instance().poolSize());

        // Populate the pools with db connections
        for (int i = 0; i < MantisApp::instance().poolSize(); ++i)
        {
            switch (MantisApp::instance().dbType())
            {
            case DbType::SQLITE:
                {
                    // For SQLite, lets explicitly define location and name of the database
                    // we intend to use within the `dataDir`
                    auto sqlite_db_path = joinPaths(MantisApp::instance().dataDir(), "mantis.db").string();
                    soci::session& sql = m_connPool->at(i);

                    auto sqlite_conn_str = std::format(
                        "db={} timeout=30 shared_cache=true synchronous=normal foreign_keys=on", sqlite_db_path);
                    sql.open(soci::sqlite3, sqlite_conn_str);
                    sql.set_logger(new MantisLoggerImpl()); // Set custom query logger

                    // Log SQL insert values in DevMode only!
                    if (MantisApp::instance().isDevMode())
                        sql.set_query_context_logging_mode(soci::log_context::always);
                    else
                        sql.set_query_context_logging_mode(soci::log_context::on_error);

                    // Open SQLite in WAL mode, helps in enabling multiple readers, single writer
                    sql << "PRAGMA journal_mode=WAL";
                    sql << "PRAGMA wal_autocheckpoint=500"; // Checkpoint every 500 pages

                    break;
                }
            case DbType::PSQL:
                {
#if MANTIS_HAS_POSTGRESQL
                    // Connection Options
                    ///> Basic: "dbname=mydb user=scott password=tiger"
                    ///> With Host: "host=localhost port=5432 dbname=test user=postgres password=postgres");
                    ///> With Config: "dbname=mydatabase user=myuser password=mypass singlerows=true"

                    soci::session& sql = m_connPool->at(i);
                    sql.open(soci::postgresql, conn_str);
                    sql.set_logger(new MantisLoggerImpl()); // Set custom query logger

                    // Log SQL insert values in DevMode only!
                    if (MantisApp::instance().isDevMode())
                        sql.set_query_context_logging_mode(soci::log_context::always);
                    else
                        sql.set_query_context_logging_mode(soci::log_context::on_error);

                    break;
#else
                    Log::warn("Database Connection for `PostgreSQL` has not been implemented yet!");
                    return false;
#endif
                }
            case DbType::MYSQL: {
                Log::warn("Database Connection for `MySQL` not implemented yet!");
                return false;
            }

            // For other DB types
            // Connect to the database URL, no checks here,
            // make your damn checks to ensure the URL is VALID!
            // TODO maybe add checks?

            default:
                Log::warn("Database Connection to `{}` Not Implemented Yet!", conn_str);
                return false;
            }
        }
    }

    catch (const soci::soci_error& e)
    {
        Log::critical("Database Connection soci::error: {}", e.what());
        return false;
    }

    catch (const std::exception& e)
    {
        Log::critical("Database Connection std::exception: {}", e.what());
        return false;
    }

    catch (...)
    {
        Log::critical("Database Connection Failed: Unknown Error");
        return false;
    }

    if (MantisApp::instance().dbType() == DbType::SQLITE)
    {
        // Write checkpoint out
        writeCheckpoint();
    }

    return true;
}

void mantis::DatabaseUnit::disconnect() const
{
    // Write checkpoint out
    writeCheckpoint();

    // Pool size cast
    const auto pool_size = static_cast<size_t>(MantisApp::instance().poolSize());
    // Close all sessions in the pool
    for (std::size_t i = 0; i < pool_size; ++i)
    {
        if (soci::session& sess = m_connPool->at(i); sess.is_connected())
        {
            // Check if session is connected
            sess.close();
        }
    }
}

bool mantis::DatabaseUnit::migrate() const
{
    // Create system tables as follows:
    // __tables for managing user tables & schema
    // __admins for managing system admin users
    // __settings for managing settings in a key - value fashion

    try
    {
        const auto sql = session();
        soci::transaction tr{*sql};

        // Create admin table, for managing and auth for admin accounts
        AdminTable admin;
        admin.name = "__admins";
        *sql << admin.to_sql();

        // Create and manage other db tables, keeping track of access rules, schema, etc.!
        SystemTable tables;
        tables.name = "__tables";
        tables.fields.emplace_back("name", FieldType::STRING, true, false, true);
        tables.fields.emplace_back("type", FieldType::STRING, true, false, true);
        tables.fields.emplace_back("schema", FieldType::STRING, true, false, true);
        tables.fields.emplace_back("has_api", FieldType::UINT8, true, false, true);
        *sql << tables.to_sql();

        // A Key - Value settings store, where the key is hashed as the table id
        SystemTable _sys;
        _sys.name = "__settings";
        _sys.fields.emplace_back("value", FieldType::JSON, true, false, true);
        *sql << _sys.to_sql();

        // Commit changes
        tr.commit();

        // Enforce migration once settings object is created!
        MantisApp::instance().settings().migrate();

        return true;
    }
    catch (std::exception& e)
    {
        Log::critical("Create System Tables Failed: {}", e.what());
    }
    return false;
}

std::shared_ptr<soci::session> mantis::DatabaseUnit::session() const
{
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

void mantis::DatabaseUnit::writeCheckpoint() const
{
    // Enable this write checkpoint for SQLite databases ONLY
    if (MantisApp::instance().dbType() == DbType::SQLITE)
    {
        try
        {
            // Write out the WAL data to db file & truncate it
            if (const auto sql = session(); sql->is_connected())
            {
                *sql << "PRAGMA wal_checkpoint(TRUNCATE)";
            }
        }
        catch (std::exception& e)
        {
            Log::critical("Database Connection SOCI::Error: {}", e.what());
        }
    }
}
