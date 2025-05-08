#ifndef MANTIS_DATABASE_H
#define MANTIS_DATABASE_H

#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Forward declare the App class
class MantisApp;

class Database
{
public:
    Database(std::shared_ptr<MantisApp> app);
    ~Database() = default;

    bool EnsureDatabaseSchemaLoaded() const;

private:
    std::shared_ptr<MantisApp> m_app;
};

#endif // MANTIS_DATABASE_H