//
// Created by allan on 08/05/2025.
//

#ifndef MANTIS_H
#define MANTIS_H

#include <string>
#include <filesystem>
#include <anyoption.h>

#include "mantis/core/logger.h"
#include "mantis/core/database.h"
#include "mantis/api/servermgr.h"

namespace fs = std::filesystem;

namespace Mantis
{
    class MantisApp: std::enable_shared_from_this<MantisApp>
    {
    public:
        MantisApp();
        ~MantisApp();

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

        int Start() const;
        int Start(const std::string& host , const int& port) const;
        int Stop() const;

        // Access the server object
        [[nodiscard]] std::shared_ptr<ServerMgr> GetSvrMgr() const;

        // Access the AnyOption CMD Arg Parser
        [[nodiscard]] std::shared_ptr<AnyOption> GetCmdParser() const;

        // Access the Database Manager
        [[nodiscard]] std::shared_ptr<DatabaseMgr> GetDbMgr() const;

        // Setters
        // Call this before starting the Server, else, will be ignored
        void SetPort(const int& port) const;
        void SetHost(const std::string& host) const;

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
        std::shared_ptr<AnyOption>      m_opts;
        std::shared_ptr<DatabaseMgr>    m_dbMgr;
        std::shared_ptr<ServerMgr>      m_svrMgr;

        // Directories
        std::string     m_publicDir;
        std::string     m_dataDir;
    };
}

#endif //MANTIS_H
