#ifndef MANTIS_DATABASE_H
#define MANTIS_DATABASE_H

#include <memory>
#include <iostream>
#include <mantis/core/models.h>
#include <SQLiteCpp/SQLiteCpp.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace Mantis
{
    // Forward declare the App class
    class MantisApp;

    class Database
    {
    public:
        explicit Database(const MantisApp& app);
        ~Database() = default;

        bool OpenDatabase();

        bool EnsureDatabaseSchemaLoaded() const;

        bool CreateSystemTables() const;

        std::shared_ptr<SQLite::Database> Db() const;

    private:
        std::string GenerateSystemDatabaseSchema() const;

    private:
        std::shared_ptr<MantisApp> m_app;
        std::shared_ptr<SQLite::Database> m_db;

    };
}
#endif // MANTIS_DATABASE_H