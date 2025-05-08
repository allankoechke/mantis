//
// Created by allan on 08/05/2025.
//

#ifndef MANTIS_H
#define MANTIS_H

#include <httplib.h>
#include <string>
#include <anyoption.h>
#include <filesystem>

#include "mantis/core/database.h"
#include "mantis/api/server.hpp"

/*
private members
    - port
    - ip
Router & Middleware
Database

*/


namespace fs = std::filesystem;

class MantisApp
{
public:
    MantisApp();
    ~MantisApp() = default;

    int ProcessCMD(int argc, char *argv[]);

    int Start();
    int Start(const std::string& host , const int& port);
    int Stop() const;

    // GETTERS
    // --------------------- //

    // Access the server object
    [[nodiscard]] std::shared_ptr<httplib::Server> Server() const;

    // Access the AnyOption CMD Arg Parser
    [[nodiscard]] std::shared_ptr<AnyOption> CmdParser() const;

    // Access the AnyOption CMD Arg Parser
    [[nodiscard]] std::shared_ptr<Database> Db() const;

    // Setters
    // Call this before starting the Server, else, will be ignored
    void SetPort(const int& port);
    void SetHost(const std::string& host);
    void SetPublicDir(const std::string& dir);
    void SetDataDir(const std::string& dir);

    // Utility Path Functions
    static fs::path ResolvePath(const std::string& input_path);
    static bool CreateDirs(const fs::path& path);

private:
    // Ensure the directory structure used/required is created,
    // if it fails, lets terminate the app
    [[nodiscard]] bool EnsureDirsAreCreated() const;

private:
    std::shared_ptr<httplib::Server> m_svr;
    std::shared_ptr<AnyOption> m_opts;
    std::shared_ptr<Database> m_db;

    // Server port & host
    int m_port;
    std::string m_host;

    // Directories
    std::string m_publicDir;
    std::string m_dataDir;

    // Database specifics
    const std::string db_name = "data.db";
};

#endif //MANTIS_H
