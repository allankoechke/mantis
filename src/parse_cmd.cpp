//
// Created by codeart on 10/11/2025.
//

#include "../include/mantisbase/mantisbase.h"
#include "../include/mantisbase/core/logs_mgr.h"
#include "../include/mantisbase/mantis.h"

#include <argparse/argparse.hpp>

#include "mantisbase/core/models/validators.h"

namespace mantis {
    void MantisBase::parseArgs() {
        // Main program parser with global arguments
        argparse::ArgumentParser program("mantisapp", appVersion());
        program.add_argument("--database", "-d")
                .nargs(1)
                .help("<type> Database type ['SQLITE', 'PSQL', 'MYSQL'] (default: SQLITE)");
        program.add_argument("--connection", "-c")
                .nargs(1)
                .help("<conn> Database connection string.");
        program.add_argument("--dataDir")
                .nargs(1)
                .help("<dir> Data directory (default: ./data)");
        program.add_argument("--publicDir")
                .nargs(1)
                .help("<dir> Static files directory (default: ./public).");
        program.add_argument("--scriptsDir")
                .nargs(1)
                .help("<dir> JS script files directory (default: ./scripts).");
        program.add_argument("--dev").flag();

        // Serve subcommand
        argparse::ArgumentParser serve_command("serve");
        serve_command.add_argument("--port", "-p")
                .default_value(7070)
                .scan<'i', int>()
                .help("<port> Server Port (default: 7070)");
        serve_command.add_argument("--host", "-h")
                .nargs(1)
                .default_value("0.0.0.0")
                .help("<host> Server Host (default: 0.0.0.0)");
        serve_command.add_argument("--poolSize")
                .scan<'i', int>()
                .help("<pool size> Size of database connection pools >= 1");

        // Admins subcommand with nested subcommands
        argparse::ArgumentParser admins_command("admins");
        // Create mutually exclusive group for --add and --rm
        auto &group = admins_command.add_mutually_exclusive_group(true);
        group.add_argument("--add")
                .nargs(1)
                .help("<email> Add a new admin user.");
        group.add_argument("--rm")
                .nargs(1)
                .help("<email/id> Remove existing admin user.");

        // Migrations subcommand with nested subcommands
        argparse::ArgumentParser migrations_command("migrate");
        admins_command.add_argument("--up")
                .nargs(1)
                .help("<file> Initiate Migration from .json file.");
        admins_command.add_argument("--down")
                .nargs(1)
                .help(".");

        // Migrations subcommand with nested subcommands
        argparse::ArgumentParser sync_command("sync");

        // Add main subparsers
        program.add_subparser(serve_command);
        program.add_subparser(admins_command);
        program.add_subparser(migrations_command);
        program.add_subparser(sync_command);

        try {
            // Create a vector of `const char*` pointing to the owned `std::string`s
            std::vector<const char *> argv;
            argv.reserve(m_cmdArgs.size());
            for (const auto &arg: m_cmdArgs) {
                argv.push_back(arg.c_str());
            }

            // Parse safely — strings are now owned by `m_cmdArgs`
            program.parse_args(static_cast<int>(argv.size()), argv.data());
        } catch (const std::exception &err) {
            std::cerr << err.what() << std::endl;
            std::stringstream ss;
            ss << program;
            logger::trace("{}", ss.str());
            quit(1, err.what());
        }

        // Get main program args
        auto db = program.present<std::string>("--database").value_or("sqlite");
        const auto connString = program.present<std::string>("--connection").value_or("");
        const auto dataDir = program.present<std::string>("--dataDir").value_or("data");
        const auto pubDir = program.present<std::string>("--publicDir").value_or("public");
        const auto scriptsDir = program.present<std::string>("--scriptsDir").value_or("scripts");

        // Set trace mode if flag is set
        if (program.get<bool>("--dev")) {
            // Print developer messages - set it to trace for now
            logger::setLogLevel(LogLevel::TRACE);
            m_isDevMode = true;
        }

        // If directory paths are not valid, we default back to the
        // default directory for the respective items (`public`, `data` and `scripts`)
        // relative to the application binary.
        const auto pub_dir = dirFromPath(pubDir);
        setPublicDir(pub_dir.empty() ? dirFromPath("public") : pub_dir);

        const auto data_dir = dirFromPath(dataDir);
        setDataDir(data_dir.empty() ? dirFromPath("data") : data_dir);

        const auto scripts_dir = dirFromPath(scriptsDir);
        setScriptsDir(scripts_dir.empty() ? dirFromPath("scripts") : scripts_dir);

        logger::trace("Mantis Configured Paths:\n\t/data: {}\n\t/public: {}\n\t/scripts: {}",
                   data_dir, pub_dir, scripts_dir);

        // Ensure objects are first created, taking into account the cmd args passed in
        // esp. the directory paths
        init_units();

        // Convert db type to lowercase and set the db type
        toLowerCase(db);
        if (db == "sqlite" || db == "sqlite3") {
            setDbType("sqlite3");
        } else if (db == "mysql") {
            setDbType(db);
        } else if (db == "psql" || db == "postgresql" || db == "postgres") {
            setDbType("postgresql");
        } else {
            quit(-1, std::format("Backend Database `{}` is unsupported!", db));
        }

        // Initialize database connection & Migration
        if (!m_database->connect(connString)) {
            // Connection to database failed
            quit(-1, "Database connection failed, exiting!");
        }
        if (!m_database->migrate()) {
            quit(-1, "Database migration failed, exiting!");
        }

        if (!m_database->isConnected()) {
            logger::critical("Database was not opened");
            quit(-1, "Database opening failed!");
        }

        // Check which commands were used
        if (program.is_subcommand_used("serve")) {
            const auto host = serve_command.get<std::string>("--host");
            const auto port = serve_command.get<int>("--port");

            int default_pool_size = m_dbType == "sqlite3" ? 4 :  10;
            const auto pools = serve_command.present<int>("--poolSize").value_or(default_pool_size);

            setHost(host);
            setPort(port);
            setPoolSize(pools > 0 ? pools : 1);

            // Set the serve flag to true, will be checked later before
            // running the listen on port & host above.
            m_toStartServer = true;
        } else if (program.is_subcommand_used("admins")) {
            const auto admin_user = admins_command.present<std::vector<std::string> >("--add")
                    .value_or(std::vector<std::string>{});

            if (admins_command.is_used("--add")) {
                if (const auto ev = Validators::validatePreset("email", admin_user.at(0));
                    !ev.at("error").get<std::string>().empty()) {
                    logger::critical("Error validating admin email: {}", ev.at("error").get<std::string>());
                    quit(-1, "Email validation failed!");
                }

                // Get password from user then validate it!
                auto password = trim(getUserValueSecurely("Getting Admin Password"));
                if (auto c_password = trim(getUserValueSecurely("Confirm Admin Password"));
                    password != c_password) {
                    logger::critical("Passwords do not match!");
                    quit(-1, "Passwords do not match!");
                }

                // Validate password against regex stored
                if (const auto ev = Validators::validatePreset("password", password);
                    !ev.at("error").get<std::string>().empty()) {
                    logger::critical("Error validating email: {}", ev.at("error").get<std::string>());
                    quit(-1, "Email validation failed!");
                }

                try {
                    auto admin_entity = entity("__admins");

                    // Create new admin user
                    json new_admin{{"email", admin_user.at(0)}, {"password", password}};
                    const auto admin_user = admin_entity.create(new_admin);

                    // Admin User was created!
                    logger::info("Admin account created, use '{}' to access the `/admin` dashboard.",
                              admin_user.at("email").get<std::string>());
                    quit(0, "");
                } catch (const std::exception &e) {
                    logger::critical("Failed to created Admin user: {}", e.what());
                    quit(-1, e.what());
                }
            } else if (admins_command.is_used("--rm")) {
                const auto admin_email_or_id = admins_command.present<std::string>("--rm")
                        .value_or("");
                if (trim(admin_email_or_id).length() < 5) {
                    logger::critical("Invalid Admin email or id provided!");
                    quit(1, "");
                }

                auto admin_entity = entity("__admins");
                auto resp = admin_entity.queryFromCols(admin_email_or_id, {"id", "email"});
                if (!resp.has_value()) {
                    logger::critical("Failed to get admin account matching `id` or `email` provided '{}'",
                                  admin_email_or_id);
                    quit(-1, "");
                }

                try {
                    const auto data = resp.value().at("data").get<json>();
                    logger::trace("Admin Data: {}", data.dump());
                    admin_entity.remove(data.at("id").get<std::string>());
                    logger::info("Admin removed successfully.");
                    quit(0, "");
                } catch (const std::exception &e) {
                    logger::critical("Failed to remove admin account: {}", e.what());
                }

                quit(-1, "");
            }
        } else if (program.is_subcommand_used("migrate")) {
            // Do migration stuff here
            logger::info("Migration CMD support has not been implemented yet! ");
        } else if (program.is_subcommand_used("sync")) {
            // Do sync actions
            logger::info("Sync CMD support has not been implemented yet!");
        }
    }
}
