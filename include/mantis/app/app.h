//
// Created by allan on 16/05/2025.
//

#ifndef APP_H
#define APP_H

#include <string>
#include <filesystem>
#include <argparse/argparse.hpp>

#include <builtin_features.h>
#include "../core/jwtprovider.h"
#include "../core/expr_evaluator.h"

// For password management ... // TODO get a proper library
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif


namespace fs = std::filesystem;

namespace mantis
{
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

        int run() const;
        void close() const;
        static int quit(const int& exitCode = 0, const std::string& reason = "Something went wrong!");

        int port() const;
        void setPort(const int& port);

        int poolSize() const;
        void setPoolSize(const int& pool_size);

        std::string host() const;
        void setHost(const std::string& host);

        std::string publicDir() const;
        void setPublicDir(const std::string& dir);

        std::string dataDir() const;
        void setDataDir(const std::string& dir);

        DbType dbType() const;
        void setDbType(const DbType& dbType);

        static std::string jwtSecretKey();

        [[nodiscard]] DatabaseUnit& db() const;
        [[nodiscard]] LoggingUnit& log() const;
        [[nodiscard]] HttpUnit& http() const;
        [[nodiscard]] argparse::ArgumentParser& cmd() const;
        [[nodiscard]] Router& router() const;
        [[nodiscard]] Validator& validators() const;
        [[nodiscard]] ExprEvaluator& evaluator() const;

    private:
        void parseArgs(int argc, char** argv);
        void initialize();
        [[nodiscard]] bool ensureDirsAreCreated() const;

        static std::string getUserValueSecurely(const std::string& prompt);

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
