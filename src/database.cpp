//
// Created by allan on 08/05/2025.
//

#include <mantis/core/database.h>
#include <mantis/mantis.h>

Database::Database(std::shared_ptr<MantisApp> app)
    : m_app(app)
{}

bool Database::EnsureDatabaseSchemaLoaded() const
{
    return true;
}