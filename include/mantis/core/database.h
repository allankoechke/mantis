#ifndef MANTIS_DATABASE_H
#define MANTIS_DATABASE_H

#include <memory>
#include <soci/soci.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace Mantis
{
    // Forward declare the App class
    class MantisApp;

    // DatabaseType enum for supported Databases
    // Check `soci` docs, this is cut down for development
    // TODO add other database types later
    typedef enum DatabaseType
    {
        SQLITE = 0,
        PSQL,
        MYSQL
    } DatabaseType;

    // Database
    class DatabaseMgr
    {
    public:
        explicit DatabaseMgr(const MantisApp& app);
        ~DatabaseMgr() = default;

        // Convenient func to call for initializing SQLite DB
        [[nodiscard]] bool DbInit();

        // If DB type is already set, invoke this with the connection string
        [[nodiscard]] bool DbInit(const std::string& connectionString);

        // Full connection link, pass in database type and connection string if applicable
        [[nodiscard]] bool DbInit(const DatabaseType& dbType, const std::string& connectionString);

        // Get a handle to a database session from a pool
        [[nodiscard]] std::shared_ptr<soci::session> DbSession() const;

        // Setter func for connection pool size
        void SetPoolSize(const int& poolSize=1);

        // Setter func for database type
        void SetDatabaseType(const DatabaseType& dbType);

        bool EnsureDatabaseSchemaLoaded() const;
        bool CreateSystemTables() const;


    private:
        std::string GenerateSystemDatabaseSchema() const;

        // Private Member Variable
        std::shared_ptr<MantisApp> m_app;
        std::shared_ptr<soci::session> m_db;

        // Database Type Selected
        int m_poolSize;
        DatabaseType m_databaseType;
        std::unique_ptr<soci::connection_pool> m_dbPool;

    };
}
#endif // MANTIS_DATABASE_H