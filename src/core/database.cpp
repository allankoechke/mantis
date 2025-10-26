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

namespace mantis
{
    DatabaseUnit::DatabaseUnit() : m_connPool(nullptr)
    {
    }

    DatabaseUnit::~DatabaseUnit()
    {
        disconnect();
    }

    bool DatabaseUnit::connect([[maybe_unused]] const DbType backend, const std::string& conn_str)
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
                case DbType::MYSQL:
                    {
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

    void DatabaseUnit::disconnect() const
    {
        // Write checkpoint out
        writeCheckpoint();

        // Pool size cast
        const auto pool_size = static_cast<size_t>(MantisApp::instance().poolSize());
        // Close all sessions in the pool
        for (std::size_t i = 0; i < pool_size; ++i)
        {
            try
            {
                if (soci::session& sess = m_connPool->at(i); sess.is_connected())
                {
                    // Check if session is connected
                    sess.close();
                }
            }
            catch (const soci::soci_error& e)
            {
                Log::critical("Database disconnection soci::error at index `{}`: {}", i, e.what());
            }
        }
    }

    bool DatabaseUnit::migrate() const
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

    std::shared_ptr<soci::session> DatabaseUnit::session() const
    {
        return std::make_shared<soci::session>(*m_connPool);
    }

    soci::connection_pool& DatabaseUnit::connectionPool() const
    {
        return *m_connPool;
    }

    nlohmann::json DatabaseUnit::rowToJson(const soci::row& r)
    {
        nlohmann::json j;

        for (std::size_t i = 0; i < r.size(); ++i)
        {
            const soci::column_properties& props = r.get_properties(i);
            const std::string& name = props.get_name();

            // Check for NULL values
            if (r.get_indicator(i) == soci::i_null)
            {
                j[name] = nullptr;
                continue;
            }

            // Map based on database type
            switch (props.get_db_type())
            {
            case soci::db_string:
                j[name] = r.get<std::string>(i);
                break;
            case soci::db_int8:
                j[name] = r.get<int8_t>(i);
                break;
            case soci::db_uint8:
                j[name] = r.get<uint8_t>(i);
                break;
            case soci::db_int16:
                j[name] = r.get<int16_t>(i);
                break;
            case soci::db_uint16:
                j[name] = r.get<uint16_t>(i);
                break;
            case soci::db_int32:
                j[name] = r.get<int32_t>(i);
                break;
            case soci::db_uint32:
                j[name] = r.get<uint32_t>(i);
                break;
            case soci::db_int64:
                j[name] = r.get<int64_t>(i);
                break;
            case soci::db_uint64:
                j[name] = r.get<uint64_t>(i);
                break;
            case soci::db_double:
                j[name] = r.get<double>(i);
                break;
            case soci::db_date:
                {
                    std::tm tm = r.get<std::tm>(i);
                    char buf[32];
                    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
                    j[name] = buf;
                }
                break;
            default:
                throw soci::soci_error("Unsupported data type for JSON conversion");
            }
        }

        return j;
    }

    bool DatabaseUnit::isConnected() const
    {
        if (m_connPool == nullptr) return false;

        const auto sql = session();
        return sql->is_connected();
    }

    void DatabaseUnit::registerDuktapeMethods()
    {
        const auto ctx = MantisApp::instance().ctx();

        // DatabaseUnit methods
        dukglue_register_property(ctx, &DatabaseUnit::isConnected, nullptr, "connected");
        dukglue_register_method(ctx, &DatabaseUnit::session, "session");
        dukglue_register_method_varargs(ctx, &DatabaseUnit::query, "query");

        // soci::session methods
        dukglue_register_method(ctx, &soci::session::close, "close");
        dukglue_register_method(ctx, &soci::session::reconnect, "reconnect");
        dukglue_register_property(ctx, &soci::session::is_connected, nullptr, "connected");
        dukglue_register_method(ctx, &soci::session::begin, "begin");
        dukglue_register_method(ctx, &soci::session::commit, "commit");
        dukglue_register_method(ctx, &soci::session::rollback, "rollback");
        dukglue_register_method(ctx, &soci::session::get_query, "getQuery");
        dukglue_register_method(ctx, &soci::session::get_last_query, "getLastQuery");
        dukglue_register_method(ctx, &soci::session::get_last_query_context, "getLastQueryContext");
        dukglue_register_method(ctx, &soci::session::got_data, "gotData");
        // dukglue_register_method(ctx, &soci::session::get_last_insert_id, "getLastInsertId");
        dukglue_register_method(ctx, &soci::session::get_backend_name, "getBackendName");
        dukglue_register_method(ctx, &soci::session::empty_blob, "emptyBlob");
    }

    duk_ret_t DatabaseUnit::query(duk_context* ctx)
    {
        // TRACE_CLASS_METHOD();

        // Get number of arguments
        const int nargs = duk_get_top(ctx);

        if (nargs < 1)
        {
            Log::critical("[JS] Expected at least 1 argument (query string)");
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "Expected at least 1 argument (query string)");
            return DUK_RET_TYPE_ERROR;
        }

        // First argument is the SQL query
        const char* query = duk_require_string(ctx, 0);

        // Collect remaining arguments (bind parameters)
        soci::values vals;
        for (int i = 1; i < nargs; i++)
        {
            if (!duk_is_object(ctx, i))
            {
                Log::critical("[JS] Arguments after query must be objects.");
                duk_error(ctx, DUK_ERR_TYPE_ERROR, "Arguments after query must be objects");
                return DUK_RET_TYPE_ERROR;
            }

            // Convert JavaScript object to JSON string
            duk_dup(ctx, i); // Duplicate the object at index i
            const char* json_str = duk_json_encode(ctx, -1);
            duk_pop(ctx); // Pop the encoded string

            // Parse into nlohmann::json
            nlohmann::json json_obj = nlohmann::json::parse(json_str);
            for (auto& [key, value] : json_obj.items())
            {
                if (value.is_string())
                {
                    auto str_val = value.get<std::string>();
                    vals.set(key, str_val);
                }
                else if (value.is_number_integer())
                {
                    int int_val = value.get<int>();
                    vals.set(key, int_val);
                }
                else if (value.is_number_float())
                {
                    double double_val = value.get<double>();
                    vals.set(key, double_val);
                }
                else if (value.is_boolean())
                {
                    bool bool_val = value.get<bool>();
                    vals.set(key, bool_val);
                }
                else if (value.is_null())
                {
                    std::optional<int> val;
                    vals.set(key, val, soci::i_null);
                }
                else if (value.is_object() || value.is_array())
                {
                    vals.set(key, value);
                }
                else
                {
                    auto err = std::format("Could not cast type at {} to DB supported types.", (i - 1));
                    Log::critical("[JS] {}", err);
                    duk_error(ctx, DUK_ERR_TYPE_ERROR, err.c_str());
                    return DUK_RET_TYPE_ERROR;
                }
            }
        }

        // Get SQL Session
        auto sql = session();

        Log::trace("[JS] soci::value binding? {}", vals.get_number_of_columns());

        // Execute SQL Statement
        soci::rowset<soci::row> rs = (sql->prepare << query, soci::use(vals));

        json results = json::array();
        for (auto & r : rs)
        {
            nlohmann::json obj;
            const json row = rowToJson(r);
            results.push_back(obj);
        }

        Log::trace("[JS] Results: {}", results.dump());

        if (results.empty())
        {
            // Return null
            duk_push_null(ctx);
            return 1;
        }

        // Convert nlohmann::json to JavaScript object
        std::string results_str = results.size() == 1 ? results[0].dump() : results.dump();
        duk_push_string(ctx, results_str.c_str());
        duk_json_decode(ctx, -1);
        return 1; // Return the object
    }

    void DatabaseUnit::writeCheckpoint() const
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
} // namespace mantis
