//
// Created by allan on 16/05/2025.
//

#ifndef DATABASE_H
#define DATABASE_H

#include <memory>
#include <soci/soci.h>
#include <nlohmann/json.hpp>
#include <private/soci-mktime.h>

#include "../app/app.h"
#include "logging.h"

namespace soci {
    using json = nlohmann::json;

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
    using json = nlohmann::json;

    class MantisApp;

    class DatabaseUnit {
    public:
        explicit DatabaseUnit(MantisApp* app);

        // Initialize the connection pool & connect to specific database
        bool connect(DbType backend, const std::string& conn_str);

        // Run database migrations
        void migrate();

        // Get access to a session from the pool
        [[nodiscard]] std::shared_ptr<soci::session> session() const;

        // Access to the underlying pool if needed
        [[nodiscard]] soci::connection_pool& connectionPool() const;

        // Check if the database is connected
        [[nodiscard]] bool isConnected() const;

        static std::string tmToISODate(const std::tm& t);

    private:
        MantisApp* m_app;
        std::unique_ptr<soci::connection_pool> m_connPool;
    };

    class MantisLoggerImpl : public soci::logger_impl
    {
    public:
        void start_query(std::string const & query) override
        {
            logger_impl::start_query(query);
            Log::trace("SQL Query: {}", query);
        }

    private:
        logger_impl* do_clone() const override
        {
            return new MantisLoggerImpl();
        }
    };
}

#endif //DATABASE_H
