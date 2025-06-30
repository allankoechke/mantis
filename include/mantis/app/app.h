/**
 * @file app.h
 *
 * @brief The main application for mantis.
 *
 * Controls all other units creation, commandline parsing as well as handling application state.
 *
 * Created by allan on 16/05/2025.
 */

#ifndef APP_H
#define APP_H

#include <string>
#include <mutex>
#include <filesystem>
#include <argparse/argparse.hpp>

#include "../core/expr_evaluator.h"

// For password management ... // TODO get a proper library
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace mantis
{
    namespace fs = std::filesystem;

    class DatabaseUnit;
    class HttpUnit;
    class LoggingUnit;
    class SettingsUnit;
    class Router;
    class Validator;

    /**
     * @brief Enum for which database is currently selected
     */
    enum class DbType
    {
        SQLITE = 0x01, ///> SQLite Database
        PSQL, ///> PostGreSQL Database
        MYSQL ///> MySQL Database
    };

    /**
     * @brief Mantis entry point.
     *
     * This class handles the entrypoint to the `Mantis` world, where we can
     * set/get application flags and variables, as well as access other
     * application units. These units are:
     * - DatabaseUnit: For all database handling, @see DatabaseUnit for more information.
     * - HttpUnit: Low-level http server operation and routing. @see HttpUnit for low-level access or @see Router for a high level routing methods.
     * - LoggingUnit: For logging capabilities, @see LoggingUnit for more details.
     * - Router: High level routing wrapper on top of @see HttpUnit, @see Router for more details.
     * - Validator: A validation store using regex, @see Validator for more details.
     */
    class MantisApp
    {
    public:
        /**
         * @brief MantisApp constructor, creates a singleton instance if one is not created yet.
         * @param argc Argument count.
         * @param argv Argument values.
         */
        explicit MantisApp(int argc = 0, char** argv = nullptr);
        ~MantisApp();

        /**
         * @brief Run initialization actions for Mantis, ensuring all objects are initialized properly before use.
         */
        void init();

        /**
         * @brief Retrieve existing application instance.
         * @return A reference to the existing application instance.
         */
        static MantisApp& instance();

        /**
         * @brief Start the http server and start listening for requests.
         * @return `0` if execution was okay, else a non-zero value.
         */
        [[nodiscard]]
        int run() const;

        /**
         * @brief Close the application
         *
         * Internally, this stops running http server.
         */
        void close() const;

        /**
         * @brief Quit the running application immediately.
         * @param exitCode Exit code value
         * @param reason User-friendly reason for the exit.
         * @return `exitCode` value.
         */
        static int quit(const int& exitCode = 0, const std::string& reason = "Something went wrong!");

        /**
         * @brief Retrieve HTTP Listening port.
         * @return Http Listening Port.
         */
        [[nodiscard]] int port() const;
        /**
         * @brief Set a new port value for HTTP server
         * @param port New HTTP port value.
         */
        void setPort(const int& port);

        /**
         * @brief Retrieve the database pool size value.
         * @return SOCI's database pool size.
         */
        [[nodiscard]] int poolSize() const;
        /**
         * @brief Set the database pool size value.
         * @param pool_size New pool size value.
         */
        void setPoolSize(const int& pool_size);

        /**
         * @brief Retrieve HTTP Server host address. For instance, a host of `127.0.0.1`, `0.0.0.0`, etc.
         * @return HTTP Server Host address.
         */
        [[nodiscard]] std::string host() const;
        /**
         * @brief Update HTTP Server host address.
         * @param host New HTTP Server host address.
         */
        void setHost(const std::string& host);

        /**
         * @brief Retrieve the public static file directory.
         * @return MantisApp public directory.
         */
        [[nodiscard]] std::string publicDir() const;
        /**
         * @brief Update HTTP server static file directory.
         * @param dir New directory path.
         */
        void setPublicDir(const std::string& dir);

        /**
         * @brief Retrieves the data directory where SQLite db and files are stored.
         * @return MantisApp data directory.
         */
        [[nodiscard]] std::string dataDir() const;
        /**
         * @brief Update the data directory for MantisApp.
         * @param dir New data directory.
         */
        void setDataDir(const std::string& dir);

        /**
         * @brief Retrieves the active database type.
         * @return Selected DatabaseType enum value.
         */
        [[nodiscard]] DbType dbType() const;
        /**
         * Update the active database type for Mantis.
         * @param dbType New database type enum value.
         */
        void setDbType(const DbType& dbType);

        /**
         * @brief Retrieve the JWT secret key.
         * @return JWT Secret value.
         */
        static std::string jwtSecretKey();
        /**
         * @brief Syntactic method to enforce @see init() is run before any other executions.
         *
         * Since `init()` ensures all objects are initialized, this ensures we don't use null pointers.
         *
         * @param caller Caller function name.
         */
        void ensureInitialized(const char* caller) const;

        /**
         * Fetch the application version
         * @return Application version
         */
        static std::string appVersion();
        /// Fetch the major version
        static int appMinorVersion();
        /// Fetch the minor version
        static int appMajorVersion();
        /// Fetch the patch version
        static int appPatchVersion();

        /// Get the database unit object
        [[nodiscard]] DatabaseUnit& db() const;
        /// Get the logging unit object
        [[nodiscard]] LoggingUnit& log() const;
        /// Get the http unit object
        [[nodiscard]] HttpUnit& http() const;
        /// Get the commandline parser object
        [[nodiscard]] argparse::ArgumentParser& cmd() const;
        /// Get the router object instance.
        [[nodiscard]] Router& router() const;
        /// Get the validators unit object instance in MantisApp.
        [[nodiscard]] Validator& validators() const;
        /// Get the `cparse` expression evaluator unit object instance.
        [[nodiscard]] ExprEvaluator& evaluator() const;
        /// Get the settings unit object
        [[nodiscard]] SettingsUnit& settings() const;

    private:
        // Points to externally constructed instance (no ownership)
        static std::unique_ptr<MantisApp> s_instance;
        static std::mutex s_mutex;

        // Disable copying and moving
        MantisApp(const MantisApp&) = delete;
        MantisApp& operator=(const MantisApp&) = delete;
        MantisApp(MantisApp&&) = delete;
        MantisApp& operator=(MantisApp&&) = delete;

        // Private members
        void parseArgs();   ///> Parse command-line arguments
        void init_units();  ///> Initialize application units

        [[nodiscard]]
        bool ensureDirsAreCreated() const; /// Ensures we created all required directories
        /**
         * @brief Get user input value, especially for password inputs or any secure value
         *
         * // TODO change to a proper input library
         *
         * @param prompt The message prompt to the user
         * @return Entered user input as a string, cast accordingly!
         */
        static std::string getUserValueSecurely(const std::string& prompt);

        // Store commandline args passed in, to be used in the init phase.
        std::vector<std::string> m_cmdArgs;

        // Hold state if the instance has be initialized already!
        bool initialized = false;

        std::string m_publicDir;
        std::string m_dataDir;
        DbType m_dbType;
        std::string m_connString{};

        int m_port = 7070;
        std::string m_host = "127.0.0.1";
        int m_poolSize = 2;
        bool m_toStartServer = false;

        std::unique_ptr<DatabaseUnit> m_database;
        std::unique_ptr<LoggingUnit> m_logger;
        std::unique_ptr<HttpUnit> m_http;
        std::unique_ptr<argparse::ArgumentParser> m_opts;
        std::unique_ptr<Router> m_router;
        std::unique_ptr<Validator> m_validators;
        std::unique_ptr<ExprEvaluator> m_exprEval;
        std::unique_ptr<SettingsUnit> m_settings;
    };
}

#endif //APP_H
