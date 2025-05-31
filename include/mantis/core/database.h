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

#include "../app/app.h"
#include "private/soci-mktime.h"

using json = nlohmann::json;

namespace soci {
    // Boolean type conversion
    template <>
    struct type_conversion<bool> {
        typedef int8_t base_type;

        static void from_base(int8_t i, indicator ind, bool & b) {
            if (ind == i_null) {
                b = false;
                return;
            }
            b = (i != 0);
        }

        static void to_base(const bool & b, int8_t & i, indicator & ind) {
            i = b ? 1 : 0;
            ind = i_ok;
        }
    };

    // JSON type conversion (assuming you have a JSON class)
    template <>
    struct type_conversion<json> {
        typedef std::string base_type;

        static void from_base(const std::string & s, indicator ind, json & jb) {
            if (ind == i_null) {
                jb = json{}; // or handle null as appropriate
                return;
            }
            jb = json::parse(s); // or however you parse JSON
        }

        static void to_base(const json & json, std::string & s, indicator & ind) {
            s = json.dump(); // or however you serialize JSON
            ind = i_ok;
        }
    };
}

namespace mantis
{
    class MantisApp;

    class DatabaseUnit {
    public:
        explicit DatabaseUnit(MantisApp* app);

        // Initialize the connection pool & connect to specific database
        bool connect(const DbType backend, const std::string& conn_str);

        // Run database migrations
        void migrate();

        // Get access to a session from the pool
        [[nodiscard]] std::shared_ptr<soci::session> session() const;

        // Access to the underlying pool if needed
        [[nodiscard]] soci::connection_pool& connectionPool() const;

        // Check if the database is connected
        bool isConnected() const;

        static inline std::string tmToISODate(const std::tm& t)
        {
            char buffer[80];
            const int length = soci::details::format_std_tm(t, buffer, sizeof(buffer));
            std::string iso_string(buffer, length);
            return iso_string;
        };

    private:
        MantisApp* m_app;
        std::unique_ptr<soci::connection_pool> m_connPool;
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
