#ifndef MANTIS_DATABASE_H
#define MANTIS_DATABASE_H

#include <memory>

// Forward declare the App class
class MantisApp;

class Database
{
public:
    Database();
    ~Database() = default;

    bool EnsureDatabaseSchemaLoaded() const;

private:
    std::shared_ptr<MantisApp> m_svr;
};

#endif // MANTIS_DATABASE_H