//
// Created by allan on 16/05/2025.
//

#include "mantis/mantis.h"
#include <builtin_features.h>
#include <mantis/app/config.hpp>
#include <cmrc/cmrc.hpp>
#include <format>
#include <fstream>

#include "mantis/core/settings.h"
#include "mantis/core/fileunit.h"
#include "mantis/core/private-impl/duktape_custom_types.h"

/**
 * @brief Enforce `MantisApp` initialization before invoking member functions
 */
#define MANTIS_REQUIRE_INIT() \
    MantisApp::instance().ensureInitialized(__func__);

#define __file__ "app/app.cpp"

namespace mantis
{
    // -------------------------------------------------------------------------------- //
    // Static member definitions
    MantisApp* MantisApp::s_instance = nullptr;
    std::mutex MantisApp::s_mutex;

    // -------------------------------------------------------------------------------- //
    MantisApp::MantisApp(const int argc, char** argv)
        : m_dbType(DbType::SQLITE),
          m_startTime(std::chrono::steady_clock::now())
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_instance)
            throw std::runtime_error("MantisApp already instantiated, use MantisApp::instance() instead!");

        // Assign `this` pointer to the instance
        s_instance = this;

        // Store cmd args into our member vector
        m_cmdArgs.reserve(argc);
        for (int i = 0; i < argc; ++i)
        {
            m_cmdArgs.emplace_back(argv[i]); // copy each string over
        }

        // Initialize Default Features in cparse
        cparse::cparse_init();

        // Enable Multi Sinks
        Log::init();

        // Create a default duk_context
        m_dukCtx = duk_create_heap_default();
    }

    MantisApp::~MantisApp()
    {
        TRACE_CLASS_METHOD()

        // Terminate any shared pointers
        close();

        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_instance == this)
        {
            // Reset instance pointer
            s_instance = nullptr;
        }

        // Destroy duk context
        duk_destroy_heap(m_dukCtx);
    }

    void MantisApp::init()
    {
        // If we had init already, don't proceed!
        if (initialized) return;

        Log::info("Initializing Mantis, v{}", appVersion());

        // Set the initialized flag
        initialized = true;

        parseArgs(); // Parse args & start units
        initJSEngine(); // Initialize JS engine
    }

    int MantisApp::initAndRun()
    {
        // Initialize Mantis
        init();

        // If initialization succeeded, lets run our server
        // and return the error code if it fails
        return run();
    }

    MantisApp& MantisApp::instance()
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (!s_instance)
            throw std::runtime_error("MantisApp not yet instantiated");
        return *s_instance;
    }

    void MantisApp::parseArgs()
    {
        MANTIS_REQUIRE_INIT();

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
        auto& group = admins_command.add_mutually_exclusive_group(true);
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

        try
        {
            // Create a vector of `const char*` pointing to the owned `std::string`s
            std::vector<const char*> argv;
            argv.reserve(m_cmdArgs.size());
            for (const auto& arg : m_cmdArgs)
            {
                argv.push_back(arg.c_str());
            }

            // Parse safely — strings are now owned by `m_cmdArgs`
            program.parse_args(static_cast<int>(argv.size()), argv.data());
        }
        catch (const std::exception& err)
        {
            std::cerr << err.what() << std::endl;
            std::stringstream ss;
            ss << program;
            Log::trace("{}", ss.str());
            quit(1, err.what());
        }

        // Get main program args
        auto db = program.present<std::string>("--database").value_or("sqlite");
        const auto connString = program.present<std::string>("--connection").value_or("");
        const auto dataDir = program.present<std::string>("--dataDir").value_or("data");
        const auto pubDir = program.present<std::string>("--publicDir").value_or("public");
        const auto scriptsDir = program.present<std::string>("--scriptsDir").value_or("scripts");

        // Set trace mode if flag is set
        if (program.get<bool>("--dev"))
        {
            // Print developer messages - set it to trace for now
            Log::setLogLevel(LogLevel::TRACE);
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

        Log::info("Mantis Configured Paths:\n\t> Data Dir: {}\n\t> Public Dir: {}\n\t> Scripts Dir: {}",
                  data_dir, pub_dir, scripts_dir);

        // Ensure objects are first created, taking into account the cmd args passed in
        // esp. the directory paths
        init_units();

        // Convert db type to lowercase and set the db type
        toLowerCase(db);
        if (db == "sqlite")
        {
            setDbType(DbType::SQLITE);
        }

        else if (db == "mysql")
        {
            setDbType(DbType::MYSQL);
        }

        else if (db == "psql" || db == "postgresql" || db == "postgres")
        {
            setDbType(DbType::PSQL);
        }

        else
        {
            quit(-1, std::format("Backend Database `{}` is unsupported!", db));
        }

        // Initialize database connection & Migration
        if (!m_database->connect(m_dbType, connString))
        {
            // Connection to database failed
            quit(-1, "Database connection failed, exiting!");
        }
        if (!m_database->migrate())
        {
            quit(-1, "Database migration failed, exiting!");
        }

        if (!m_database->isConnected())
        {
            Log::critical("Database was not opened");
            quit(-1, "Database opening failed!");
        }

        // Check which commands were used
        if (program.is_subcommand_used("serve"))
        {
            const auto host = serve_command.get<std::string>("--host");
            const auto port = serve_command.get<int>("--port");

            int default_pool_size = m_dbType == DbType::SQLITE ? 4 : m_dbType == DbType::PSQL ? 10 : 1;
            const auto pools = serve_command.present<int>("--poolSize").value_or(default_pool_size);

            setHost(host);
            setPort(port);
            setPoolSize(pools > 0 ? pools : 1);

            // Set the serve flag to true, will be checked later before
            // running the listen on port & host above.
            m_toStartServer = true;
        }
        else if (program.is_subcommand_used("admins"))
        {
            const auto admin_user = admins_command.present<std::vector<std::string>>("--add")
                                                  .value_or(std::vector<std::string>{});

            // Create admin table object, we'll use it to get JSON rep for use in
            // the TableUnit construction. Similar to what we do when creating routes.
            AdminTable admin;
            admin.name = "__admins";
            admin.id = TableUnit::generateTableId("__admins");

            // Create TableUnit from admin json dump
            TableUnit t{admin.to_json()};

            if (admins_command.is_used("--add"))
            {
                if (const auto ev = validators().validate("email", admin_user.at(0));
                    !ev.at("error").get<std::string>().empty())
                {
                    Log::critical("Error validating admin email: {}", ev.at("error").get<std::string>());
                    quit(-1, "Email validation failed!");
                }

                // Get password from user then validate it!
                auto password = trim(getUserValueSecurely("Getting Admin Password"));
                if (auto c_password = trim(getUserValueSecurely("Confirm Admin Password"));
                    password != c_password)
                {
                    Log::critical("Passwords do not match!");
                    quit(-1, "Passwords do not match!");
                }

                // Validate password against regex stored
                if (const auto ev = validators().validate("password", password);
                    !ev.at("error").get<std::string>().empty())
                {
                    Log::critical("Error validating email: {}", ev.at("error").get<std::string>());
                    quit(-1, "Email validation failed!");
                }

                // Create new admin user
                json new_admin{{"email", admin_user.at(0)}, {"password", password}};
                if (const auto resp = t.create(new_admin, json::object());
                    resp.at("status").get<int>() != 201)
                {
                    Log::critical("Failed to created Admin user: {}", resp.at("error").get<std::string>());
                    quit(-1, "");
                }

                // Admin User was created!
                Log::info("Yes! Admin created successfully.");
                quit(0, "");
            }
            else if (admins_command.is_used("--rm"))
            {
                const auto admin_email_or_id = admins_command.present<std::string>("--rm")
                                                             .value_or("");
                if (trim(admin_email_or_id).length() < 5)
                {
                    Log::critical("Invalid Admin email or id provided!");
                    quit(1, "");
                }

                // Check if a record exists in db of such user ...
                Log::trace("Check if email/id [{}] exists", admin_email_or_id);
                auto resp = t.checkValueInColumns(admin_email_or_id, {"id", "email"});
                Log::trace("Admin Found Response: {}", resp.dump());
                if (!resp.at("error").get<std::string>().empty())
                {
                    Log::critical("Failed to get admin account matching id/email = {} - {}",
                                  admin_email_or_id, resp.at("error").get<std::string>());
                    quit(-1, "");
                }

                try
                {
                    const auto data = resp.at("data").get<json>();
                    Log::trace("Admin Data: {}", data.dump());
                    if (t.remove(data.at("id").get<std::string>(), json::object()))
                    {
                        Log::info("Admin removed successfully.");
                        quit(0, "");
                    }
                }
                catch (soci::soci_error& e)
                {
                    Log::critical("Failed to remove admin account: {}", e.what());
                }
                quit(-1, "");
            }
        }
        else if (program.is_subcommand_used("migrate"))
        {
            // Do migration stuff here
            Log::info("Migration CMD support has not been implemented yet! ");
        }
        else if (program.is_subcommand_used("sync"))
        {
            // Do sync actions
            Log::info("Sync CMD support has not been implemented yet!");
        }
    }

    void MantisApp::init_units()
    {
        if (!ensureDirsAreCreated())
            quit(-1, "Failed to create database directories!");

        // Create instance objects
        m_logger = std::make_unique<LoggingUnit>();
        m_exprEval = std::make_unique<ExprEvaluator>(); // depends on log()
        m_database = std::make_unique<DatabaseUnit>(); // depends on log()
        m_http = std::make_unique<HttpUnit>(); // depends on db()
        m_router = std::make_unique<Router>(); // depends on db() & http()
        m_settings = std::make_unique<SettingsUnit>(); // depends on db(), router() & http()
        m_opts = std::make_unique<argparse::ArgumentParser>();
        m_validators = std::make_unique<Validator>();
        m_files = std::make_unique<FileUnit>(); // depends on log()
    }

    int MantisApp::quit(const int& exitCode, [[maybe_unused]] const std::string& reason)
    {
        // Stop server if running
        MantisApp::instance().close();

        if (exitCode != 0)
            Log::critical("Exiting Application with Code = {}", exitCode);
        else
            Log::info("Application exiting normally!");

        std::exit(exitCode);
    }

    void MantisApp::close()
    {
        MANTIS_REQUIRE_INIT();

        // Destroy instance objects
        if (m_files) m_files.reset();
        if (m_validators) m_validators.reset();
        if (m_opts) m_opts.reset();
        if (m_settings) m_settings.reset();
        if (m_router) m_router.reset();
        if (m_http) m_http.reset();
        if (m_database) m_database.reset();
        if (m_exprEval) m_exprEval.reset();
        if (m_logger) m_logger.reset();
    }

    int MantisApp::run()
    {
        MANTIS_REQUIRE_INIT();

        if (!m_router->initialize())
            quit(-1, "Failed to initialize router!");

        // Set server start time
        m_startTime = std::chrono::steady_clock::now();

        // Load start script for Mantis
        loadStartScript();

        // If server command is explicitly passed in, start listening,
        // else, exit!
        if (m_toStartServer)
        {
            if (!m_http->listen(m_host, m_port))
                return -1;
        }

        return 0;
    }

    DatabaseUnit& MantisApp::db() const
    {
        MANTIS_REQUIRE_INIT();
        return *m_database;
    }

    LoggingUnit& MantisApp::log() const
    {
        MANTIS_REQUIRE_INIT();
        return *m_logger;
    }

    HttpUnit& MantisApp::http() const
    {
        MANTIS_REQUIRE_INIT();
        return *m_http;
    }

    argparse::ArgumentParser& MantisApp::cmd() const
    {
        MANTIS_REQUIRE_INIT();
        return *m_opts;
    }

    Router& MantisApp::router() const
    {
        MANTIS_REQUIRE_INIT();
        return *m_router;
    }

    Validator& MantisApp::validators() const
    {
        MANTIS_REQUIRE_INIT();
        return *m_validators;
    }

    ExprEvaluator& MantisApp::evaluator() const
    {
        MANTIS_REQUIRE_INIT();
        return *m_exprEval;
    }

    SettingsUnit& MantisApp::settings() const
    {
        MANTIS_REQUIRE_INIT();
        return *m_settings;
    }

    FileUnit& MantisApp::files() const
    {
        MANTIS_REQUIRE_INIT();
        return *m_files;
    }

    duk_context* MantisApp::ctx() const
    {
        return m_dukCtx;
    }

    void MantisApp::openBrowserOnStart() const
    {
        MANTIS_REQUIRE_INIT();

        // Return if flag is reset
        if (!m_launchAdminPanel) return;

        const std::string url = std::format("http://localhost:{}/admin", m_port);

#ifdef _WIN32
        std::string command = "start " + url;
#elif __APPLE__
        std::string command = "open " + url;
#elif __linux__
        std::string command = "xdg-open " + url;
#else
#error Unsupported platform
#endif

        if (int result = std::system(command.c_str()); result != 0)
        {
            Log::info("Could not open browser: {} > {}", command, result);
        }
    }

    std::chrono::time_point<std::chrono::steady_clock> MantisApp::startTime() const
    {
        return m_startTime;
    }

    bool MantisApp::isDevMode() const
    {
        return m_isDevMode;
    }

    void MantisApp::setDbType(const DbType& dbType)
    {
        MANTIS_REQUIRE_INIT();
        m_dbType = dbType;
    }

    std::string MantisApp::jwtSecretKey()
    {
        MANTIS_REQUIRE_INIT();
        // This is the default secret key, override it through environment variable
        // MANTIS_JWT_SECRET, recommended to override this key
        // TODO add commandline input for overriding the key
        return getEnvOrDefault("MANTIS_JWT_SECRET", "<our-very-secret-JWT-key>");
    }

    void MantisApp::ensureInitialized(const char* caller) const
    {
        if (!initialized)
        {
            std::cerr << "[MantisApp] Error: init() not called before use.\n";
            std::cerr << "  -> Called from: " << caller << "\n";
            throw std::runtime_error("MantisApp::init() must be called before using this method");
        }
    }

    std::string MantisApp::appVersion()
    {
        return getVersionString();
    }

    int MantisApp::appMinorVersion()
    {
        return MANTIS_VERSION_MINOR;
    }

    int MantisApp::appMajorVersion()
    {
        return MANTIS_VERSION_MAJOR;
    }

    int MantisApp::appPatchVersion()
    {
        return MANTIS_VERSION_PATCH;
    }

    DbType MantisApp::dbType() const
    {
        MANTIS_REQUIRE_INIT();
        return m_dbType;
    }

    std::string MantisApp::dbTypeByName() const
    {
        MANTIS_REQUIRE_INIT();

        switch (m_dbType)
        {
        case DbType::SQLITE: return "sqlite3";
        case DbType::PSQL: return "postgresql";
        case DbType::MYSQL: return "mysql";
        default: return "";
        }
    }

    int MantisApp::port() const
    {
        MANTIS_REQUIRE_INIT();
        return m_port;
    }

    void MantisApp::setPort(const int& port)
    {
        MANTIS_REQUIRE_INIT();
        if (port < 0 || port > 65535)
            return;

        m_port = port;
        Log::debug("Setting Server Port to {}", port);
    }

    std::string MantisApp::host() const
    {
        MANTIS_REQUIRE_INIT();
        return m_host;
    }

    void MantisApp::setHost(const std::string& host)
    {
        MANTIS_REQUIRE_INIT();
        if (host.empty())
            return;

        m_host = host;
        Log::debug("Setting Server Host to {}", host);
    }

    int MantisApp::poolSize() const
    {
        MANTIS_REQUIRE_INIT();
        return m_poolSize;
    }

    void MantisApp::setPoolSize(const int& pool_size)
    {
        MANTIS_REQUIRE_INIT();
        if (pool_size <= 0)
            return;

        m_poolSize = pool_size;
    }

    std::string MantisApp::publicDir() const
    {
        MANTIS_REQUIRE_INIT();
        return m_publicDir;
    }

    void MantisApp::setPublicDir(const std::string& dir)
    {
        MANTIS_REQUIRE_INIT();
        if (dir.empty())
            return;

        m_publicDir = dir;
    }

    std::string MantisApp::dataDir() const
    {
        MANTIS_REQUIRE_INIT();
        return m_dataDir;
    }

    void MantisApp::setDataDir(const std::string& dir)
    {
        MANTIS_REQUIRE_INIT();
        if (dir.empty())
            return;

        m_dataDir = dir;
    }

    std::string MantisApp::scriptsDir() const
    {
        MANTIS_REQUIRE_INIT();
        return m_scriptsDir;
    }

    void MantisApp::setScriptsDir(const std::string& dir)
    {
        MANTIS_REQUIRE_INIT();
        if (dir.empty())
            return;

        m_scriptsDir = dir;
    }

    inline bool MantisApp::ensureDirsAreCreated() const
    {
        MANTIS_REQUIRE_INIT();
        // Data Directory
        if (!createDirs(resolvePath(m_dataDir)))
            return false;

        if (!createDirs(resolvePath(m_publicDir)))
            return false;

        std::cout << "Scripts Dir: " << resolvePath(m_scriptsDir) << std::endl;
        if (!createDirs(resolvePath(m_scriptsDir)))
        {
            std::cout << "Error creating scripts dir" << std::endl;
            return false;
        }

        return true;
    }

    std::string MantisApp::getUserValueSecurely(const std::string& prompt)
    {
        MANTIS_REQUIRE_INIT();
        std::string password;
        Log::info("{}", prompt);
        std::cout << " [Type In] > ";

#ifdef WIN32
        char ch;
        while ((ch = _getch()) != '\r')
        {
            // Enter key
            if (ch == '\b')
            {
                // Backspace
                if (!password.empty())
                {
                    password.pop_back();
                    std::cout << "\b \b"; // Erase character from console
                }
            }
            else
            {
                password += ch;
                std::cout << '*'; // Optional: print '*' for each char
            }
        }
#else
        termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt); // get current terminal settings
        newt = oldt;
        newt.c_lflag &= ~ECHO; // disable echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt); // set new settings
        std::getline(std::cin, password); // read password
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restore old settings
#endif

        std::cout << '\n';
        return password;
    }

    void MantisApp::initJSEngine()
    {
        MANTIS_REQUIRE_INIT();
        TRACE_CLASS_METHOD();

        // ---------------------------------------------- //
        // Register `app` object
        // ---------------------------------------------- //
        // Register the singleton instance as a global
        dukglue_register_global(m_dukCtx, this, "app");

        // Properties
        dukglue_register_property(m_dukCtx, &MantisApp::host, &MantisApp::setHost, "host");
        dukglue_register_property(m_dukCtx, &MantisApp::port, &MantisApp::setPort, "port");
        dukglue_register_property(m_dukCtx, &MantisApp::poolSize, &MantisApp::setPoolSize, "poolSize");
        dukglue_register_property(m_dukCtx, &MantisApp::publicDir, &MantisApp::setPublicDir, "publicDir");
        dukglue_register_property(m_dukCtx, &MantisApp::dataDir, &MantisApp::setDataDir, "dataDir");
        dukglue_register_property(m_dukCtx, &MantisApp::isDevMode, nullptr, "devMode");
        dukglue_register_property(m_dukCtx, &MantisApp::dbTypeByName, nullptr, "dbType");
        dukglue_register_property(m_dukCtx, &MantisApp::jwtSecretKey_JSWrapper, nullptr, "secretKey");
        dukglue_register_property(m_dukCtx, &MantisApp::version_JSWrapper, nullptr, "version");

        dukglue_register_method(m_dukCtx, &MantisApp::close, "close");
        dukglue_register_method(m_dukCtx, &MantisApp::duk_db, "db");

        MantisRequest::registerDuktapeMethods();
        MantisResponse::registerDuktapeMethods();

        dukglue_register_method_varargs(m_dukCtx, &MantisApp::addRoute, "addRoute");

        // dukglue_register_method(m_dukCtx, &MantisApp::http, "http");
        // dukglue_register_method(m_dukCtx, &MantisApp::router, "router");
        // dukglue_register_method(m_dukCtx, &MantisApp::validators, "validator");
        // dukglue_register_method(m_dukCtx, &MantisApp::settings, "settings");
        // dukglue_register_method(m_dukCtx, &MantisApp::files, "files");

        // ---------------------------------------------- //
        // Register `console` object
        // ---------------------------------------------- //
        // Create console object and register methods
        duk_push_object(m_dukCtx);

        duk_push_c_function(m_dukCtx, &DuktapeImpl::nativeConsoleInfo, DUK_VARARGS);
        duk_put_prop_string(m_dukCtx, -2, "info");

        duk_push_c_function(m_dukCtx, &DuktapeImpl::nativeConsoleTrace, DUK_VARARGS);
        duk_put_prop_string(m_dukCtx, -2, "trace");

        duk_push_c_function(m_dukCtx, &DuktapeImpl::nativeConsoleInfo, DUK_VARARGS);
        duk_put_prop_string(m_dukCtx, -2, "log");

        duk_put_global_string(m_dukCtx, "console");

        // UTILS methods
        registerUtilsToDuktapeEngine();

        // DATABASE methods
        DatabaseUnit::registerDuktapeMethods();

        // HTTP methods
        // HttpUnit::registerDuktapeMethods();
    }

    void MantisApp::loadStartScript() const
    {
        // Look for index.js as the entry point
        const auto entryPoint = (fs::path(m_scriptsDir) / "index.mantis.js").string();
        loadAndExecuteScript(entryPoint);
    }

    void MantisApp::loadAndExecuteScript(const std::string& filePath) const
    {
        if (!fs::exists(fs::path(filePath)))
        {
            Log::trace("Executing a file that does not exist, path `{}`", filePath);
            return;
        }

        // If the file exists, lets load the contents and then execute
        std::ifstream file(filePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string scriptContent = buffer.str();

        try
        {
            dukglue_peval<void>(m_dukCtx, scriptContent.c_str());
        }
        catch (const DukErrorException& e)
        {
            Log::critical("Error loading file at {} \n\tError: {}", filePath, e.what());
        }
    }

    void MantisApp::loadScript(const std::string& relativePath) const
    {
        // Construct full path relative to scripts directory
        const auto fullPath = fs::path(m_scriptsDir) / relativePath;
        loadAndExecuteScript(fullPath.string());
    }

    DatabaseUnit* MantisApp::duk_db() const
    {
        return m_database.get();
    }

    duk_ret_t MantisApp::addRoute(duk_context* ctx)
    {
        // Get method (GET, POST, etc.) from argument 0
        auto method = trim(duk_require_string(ctx, 0));
        std::ranges::transform(method, method.begin(), ::toupper);
        if (method.empty() ||
            !(method == "GET" || method == "POST" || method == "PATCH" || method == "DELETE"))
        {
            duk_error(ctx, DUK_ERR_TYPE_ERROR,
                      "addRoute expects request method of type `GET`, `POST`, `PATCH` or `DELETE` only!");
            return DUK_RET_TYPE_ERROR;
        }

        // Get path from argument 1
        const auto path = trim(duk_require_string(ctx, 1));
        // TODO Catch wildcard paths as well?
        if (path.empty() || path[0] != '/')
        {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "addRoute expects route paths to be valid and start with `/`!");
            return DUK_RET_TYPE_ERROR;
        }

        // Get number of function arguments (everything after path)
        duk_idx_t n = duk_get_top(ctx);
        if (n < 3)
        {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "addRoute requires at least a handler function");
            return DUK_RET_TYPE_ERROR;
        }

        // First function (argument 2) is the handler
        if (!duk_is_callable(ctx, 2))
        {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "Argument 2 must be a callable handler function");
            return DUK_RET_TYPE_ERROR;
        }

        duk_dup(ctx, 2);
        DukValue handler = DukValue::take_from_stack(ctx);

        // Remaining functions (arguments 3+) are middleware
        std::vector<DukValue> middlewares;
        for (duk_idx_t i = 3; i < n; i++)
        {
            if (!duk_is_callable(ctx, i))
            {
                duk_error(ctx, DUK_ERR_TYPE_ERROR, "All arguments after handler must be callable functions");
                return DUK_RET_TYPE_ERROR;
            }

            duk_dup(ctx, i);
            middlewares.push_back(DukValue::take_from_stack(ctx));
        }

        if (method == "GET")
        {
            m_http->Get(path, [this, ctx, handler, middlewares](
                        const httplib::Request& req,
                        httplib::Response& res,
                        Context& context)
                        {
                            // Construct MantisRequest, MantisResponse objects
                            MantisRequest ma_req(req, context);
                            MantisResponse ma_res(res);
                            this->executeRoute(ctx, handler, middlewares, ma_req, ma_res);
                        }, {}
            );
        }
        else if (method == "POST")
        {
            m_http->Post(path, [this, ctx, handler, middlewares](
                         const httplib::Request& req,
                         httplib::Response& res,
                         Context& context)
                         {
                             // Construct MantisRequest, MantisResponse objects
                             MantisRequest ma_req(req, context);
                             MantisResponse ma_res(res);
                             this->executeRoute(ctx, handler, middlewares, ma_req, ma_res);
                         }, {}
            );
        }
        else if (method == "PATCH")
        {
            m_http->Patch(path, [this, ctx, handler, middlewares](
                          const httplib::Request& req,
                          httplib::Response& res,
                          Context& context)
                          {
                              // Construct MantisRequest, MantisResponse objects
                              MantisRequest ma_req(req, context);
                              MantisResponse ma_res(res);
                              this->executeRoute(ctx, handler, middlewares, ma_req, ma_res);
                          }, {}
            );
        }
        else if (method == "DELETE")
        {
            m_http->Delete(path, [this, ctx, handler, middlewares](
                           const httplib::Request& req,
                           httplib::Response& res,
                           Context& context)
                           {
                               // Construct MantisRequest, MantisResponse objects
                               MantisRequest ma_req(req, context);
                               MantisResponse ma_res(res);
                               this->executeRoute(ctx, handler, middlewares, ma_req, ma_res);
                           }, {}
            );
        }
        else
        {
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "Unsupported HTTP method: %s", method.c_str());
            return DUK_RET_TYPE_ERROR;
        }

        return 0; // No return value
    }

    void MantisApp::executeRoute(duk_context* ctx, const DukValue& handler, const std::vector<DukValue>& middlewares,
                                 MantisRequest& req, MantisResponse& res)
    {
        // Execute middleware functions first
        for (const auto& middleware : middlewares)
        {
            try
            {
                // Call middleware: middleware(req, res)
                // If middleware returns false, stop execution
                const bool ok = dukglue_pcall<bool>(ctx, middleware, &req, &res);

                if (!ok)
                {
                    if (res.get_status() < 400) res.set_status(500); // If error code is not explicit
                    return; // Middleware stopped the chain
                }
            }
            catch (const DukException& e)
            {
                json response;
                response["status"] = "ok";
                response["data"] = json::object();
                response["error"] = e.what();

                res.set_status(500);
                res.set_content(response.dump(), "application/json");

                Log::critical("Error Executing Middleware: {}", e.what());

                return;
            }
        }

        // Execute the handler function
        try
        {
            dukglue_pcall<void>(ctx, handler, &req, &res);
        }
        catch (const DukException& e)
        {
            json response;
            response["status"] = 500;
            response["data"] = json::object();
            response["error"] = e.what();

            res.send(500, response.dump(), "application/json");

            Log::critical("Error Executing Route {} : {}", req.get_path(), e.what());
        }
    }
}
