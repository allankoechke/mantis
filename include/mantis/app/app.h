//
// Created by allan on 16/05/2025.
//

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
    class Router;
    class Validator;

    enum class DbType
    {
        SQLITE = 0x01,
        PSQL,
        MYSQL
    };


    class MantisApp
    {
    public:
        explicit MantisApp(int argc = 0, char** argv = nullptr);
        ~MantisApp();

        void init();
        static MantisApp& instance();

        [[nodiscard]]
        int run() const;
        void close() const;
        static int quit(const int& exitCode = 0, const std::string& reason = "Something went wrong!");

        [[nodiscard]]
        int port() const;
        void setPort(const int& port);

        [[nodiscard]]
        int poolSize() const;
        void setPoolSize(const int& pool_size);

        [[nodiscard]]
        std::string host() const;
        void setHost(const std::string& host);

        [[nodiscard]]
        std::string publicDir() const;
        void setPublicDir(const std::string& dir);

        [[nodiscard]]
        std::string dataDir() const;
        void setDataDir(const std::string& dir);

        [[nodiscard]]
        DbType dbType() const;
        void setDbType(const DbType& dbType);

        static std::string jwtSecretKey();
        void ensureInitialized(const char* caller) const;

        [[nodiscard]] DatabaseUnit& db() const;
        [[nodiscard]] LoggingUnit& log() const;
        [[nodiscard]] HttpUnit& http() const;
        [[nodiscard]] argparse::ArgumentParser& cmd() const;
        [[nodiscard]] Router& router() const;
        [[nodiscard]] Validator& validators() const;
        [[nodiscard]] ExprEvaluator& evaluator() const;

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
        void parseArgs();
        void init_units();

        [[nodiscard]]
        bool ensureDirsAreCreated() const;
        static std::string getUserValueSecurely(const std::string& prompt);

        // Store commandline args passed in, to be used in
        // the init phase.
        int m_argc;
        char** m_argv;

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
    };
}

#endif //APP_H
