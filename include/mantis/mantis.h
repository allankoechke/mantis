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

namespace Mantis
{
    class MantisApp: std::enable_shared_from_this<MantisApp>
    {
    public:
        MantisApp();
        ~MantisApp() = default;

        int ProcessCMD(int argc, char *argv[]);

        /* @brief Quit the application
         *
         * Exits the running application with an `exitCode` and `reason` given
         * for the exit. Useful for terminating application if an exception
         * or an error occurred that would render the running system malformed.
         *
         * @params
         * `intCode` - Exit code for the application. For a normal exit, use `0`,
         * `reason` - Textual explanation as to why the system exitted.
         *
         * @return ignored.
         */

        static int Quit(const int& exitCode = 0, const std::string& reason = "Something went wrong!");

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
        [[nodiscard]] std::shared_ptr<DatabaseMgr> DbMgr() const;

        // Setters
        // Call this before starting the Server, else, will be ignored
        void SetPort(const int& port);
        void SetHost(const std::string& host);

        void SetPublicDir(const std::string& dir);
        std::string PublicDir() const;

        void SetDataDir(const std::string& dir);
        std::string DataDir() const;

        // Utility Path Functions
        static fs::path ResolvePath(const std::string& input_path);
        static bool CreateDirs(const fs::path& path);
        static std::string DirFromPath(const std::string& path);

    private:
        // Ensure the directory structure used/required is created,
        // if it fails, lets terminate the app
        [[nodiscard]] bool EnsureDirsAreCreated() const;

        // Private Members
        std::shared_ptr<httplib::Server> m_svr;
        std::shared_ptr<AnyOption> m_opts;
        std::shared_ptr<DatabaseMgr> m_dbMgr;

        // Server port & host
        int m_port;
        std::string m_host;

        // Directories
        std::string m_publicDir;
        std::string m_dataDir;
    };
}

#endif //MANTIS_H
