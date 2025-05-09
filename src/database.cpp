//
// Created by allan on 08/05/2025.
//

#include <memory>
#include <mantis/core/database.h>
#include <mantis/core/models.h>
#include <mantis/mantis.h>

Mantis::Database::Database(const MantisApp& app)
    : m_app(make_shared<MantisApp>(app))
{}

bool Mantis::Database::OpenDatabase()
{
    if (m_db = std::make_shared<SQLite::Database>(
        m_app->DataDir() + "/vault.db",
            SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE ))
        return true;

    return false;
}

bool Mantis::Database::EnsureDatabaseSchemaLoaded() const
{
    std::cout << CreateSystemTables() << std::endl;
    std::cout << GenerateSystemDatabaseSchema() << std::endl;
    return true;
}

bool Mantis::Database::CreateSystemTables() const
{
    // Create system tables as follows:
    // __tables for managing user tables & schema
    // __admins for managing system admin users
    // __logs * optional for logs
    // Any other?

    try
    {
        // db.exec("DROP TABLE IF EXISTS test");

        // Begin transaction
        SQLite::Transaction transaction(*m_db);

        // __admin table
        stringstream ss;
        ss << "CREATE TABLE IF NOT EXISTS __admin";
        ss << " (id INTEGER PRIMARY KEY AUTOINCREMENT, email TEXT, password TEXT,";
        ss << " created TEXT, updated TEXT)";

        m_db->exec(ss.str());

        // int nb = db.exec("INSERT INTO test VALUES (NULL, \"test\")");
        // std::cout << "INSERT INTO test VALUES (NULL, \"test\")\", returned " << nb << std::endl;

        // Commit transaction
        transaction.commit();

        return true;
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }

    return false;
}

std::shared_ptr<SQLite::Database> Mantis::Database::Db() const
{
    return m_db;
}

std::string Mantis::Database::GenerateSystemDatabaseSchema() const
{
    Mantis::BaseTable sysTableSchema;
    sysTableSchema.name = "__tables";
    sysTableSchema.enableSync = true;
    sysTableSchema.id = "";
    sysTableSchema.type = Mantis::Base;

    return sysTableSchema.to_sql();
}
