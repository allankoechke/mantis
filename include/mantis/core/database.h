//
// Created by allan on 16/05/2025.
//

#ifndef DATABASE_H
#define DATABASE_H

#include <optional>
#include <vector>
#include <memory>
#include <soci/soci.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


namespace mantis
{
    class MantisApp;

    class DatabaseUnit {
    public:
        explicit DatabaseUnit(MantisApp* app);
        bool connect(const std::string& backend, const std::string& conn_str);
        void migrate();

        soci::session& session();

    private:
        MantisApp* m_app;
        std::unique_ptr<soci::session> m_sql;
        // std::shared_ptr<UserCrud> user_crud_;

        soci::session m_db;
        soci::connection_pool m_dbPool;
    };
}

// class UserCrud : public CrudInterface<User> {
// public:
//     explicit UserCrud(soci::session& sql);
//     bool create(const User& entity) override;
//     std::optional<User> read(int id) override;
//     bool update(int id, const User& entity) override;
//     bool remove(int id) override;
//     std::vector<User> list() override;
//
// private:
//     soci::session& sql_;
// };
//
// std::shared_ptr<UserCrud> users();
//
// template <typename T>
// class CrudInterface {
// public:
//     virtual ~CrudInterface() = default;
//     virtual bool create(const T& entity) = 0;
//     virtual std::optional<T> read(int id) = 0;
//     virtual bool update(int id, const T& entity) = 0;
//     virtual bool remove(int id) = 0;
//     virtual std::vector<T> list() = 0;
// };



#endif //DATABASE_H
