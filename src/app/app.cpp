//
// Created by allan on 16/05/2025.
//

#include "../../include/mantis/mantis.h"

mantis::MantisApp::MantisApp(int argc, char** argv)
    :m_dbType(DbType::SQLITE)
{
    // Initialize Default Features in cparse
    cparse::cparse_init();

    // Enable Multi Sinks
   Log::init();

    // Set initial public directory
    auto dir = dirFromPath("./public");
    setPublicDir(dir);

    // Set initial data directory
    dir = dirFromPath("./data");
    setDataDir(dir);

    initialize();
    parseArgs(argc, argv);
}

mantis::MantisApp::~MantisApp()
{
    Log::close();
}

void mantis::MantisApp::parseArgs(const int argc, char** argv) {
    // Main program parser with global arguments
    argparse::ArgumentParser program("mantisapp");
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

    // Print the help page
    std::cout << program << std::endl;

    try
    {
        program.parse_args(argc, argv);
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
    const auto m_connString = program.present<std::string>("--connection").value_or("");
    const auto dataDir = program.present<std::string>("--dataDir").value_or("./data");
    const auto pubDir = program.present<std::string>("--publicDir").value_or("./public");

    // Set trace mode if flag is set
    if (const auto isDev = program.get<bool>("--dev"))
    {
        // Print developer messages - set it to trace for now
        Log::setLogLevel(LogLevel::TRACE);
    }

    // TODO validate directory paths
    const auto pub_dir = dirFromPath(pubDir);
    setPublicDir(pub_dir);

    const auto data_dir = dirFromPath(dataDir);
    setDataDir(data_dir);

    toLowerCase(db);
    if      (db == "sqlite")    setDbType(DbType::SQLITE);
    else if (db == "mysql")     setDbType(DbType::MYSQL);
    else if (db == "psql")      setDbType(DbType::PSQL);
    else    quit(-1, "Backend Database '" + db + "' is unknown!");

    // Initialize database connection & Migration
    m_database->connect(m_dbType, m_connString);
    m_database->migrate();

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

        setHost(host);
        setPort(port);
        std::cout << "- " << host << ":" << port << std::endl;
    }
    else if (program.is_subcommand_used("admins"))
    {
        const auto admin_user = admins_command.present<std::vector<std::string>>("--add")
                                              .value_or(std::vector<std::string>{});

        // You'll need to implement or use a library for secure password input
        const auto getPassword = [&] () ->std::string {
            // This would need to use platform-specific code or a library
            // like termios on Unix/Linux or conio.h on Windows
            // to hide password input from terminal display
            std::string password;
            std::cout << "Enter password: ";
            // Secure input implementation needed here
            std::getline(std::cin, password);
            return password;
        };

        TableUnit t{this, "__admins", TableUnit::generateTableId("__admins"), "auth"};
        // json admin{{"email", admin_user.at(0)}, }
        // t.create();

        if (admins_command.is_used("--add"))
        {

            // Handle creating user account here ...
        }

        else if (admins_command.is_used("--rm"))
        {
            const auto admin_email_or_id = admins_command.present<std::string>("--rm")
                                                     .value_or("");
            if (trim(admin_email_or_id).length() < 5)
            {
                quit(1, "Invalid Admin email or password provided!");
            }

            // Handle deleting user account here ...
        }
    }
    else if (program.is_subcommand_used("migrate"))
    {
        // Do migration stuff here
    }
    else if (program.is_subcommand_used("sync"))
    {
        // Do sync actions
    }
}

void mantis::MantisApp::initialize() {
    if (!ensureDirsAreCreated())
        quit(-1, "Failed to create database directories!");

    // Create instance objects
    m_exprEval      = std::make_unique<ExprEvaluator>();
    m_logger        = std::make_unique<LoggingUnit>();
    m_database      = std::make_unique<DatabaseUnit>(this);
    m_http          = std::make_unique<HttpUnit>();
    m_opts          = std::make_unique<argparse::ArgumentParser>();
    m_router        = std::make_unique<Router>(this);
    m_validators    = std::make_unique<Validator>();
}

int mantis::MantisApp::quit(const int& exitCode, [[maybe_unused]] const std::string& reason)
{
    // Do some cleanup if need be ...
    // m_db->CloseIfOpened();
    // m_svr->CloseIfOpened();

    if (exitCode != 0)
        Log::critical("Exiting Application with Code = {}", exitCode);
    else
        Log::info("Application exiting normally!");

    std::exit(exitCode);
}

void mantis::MantisApp::close() const
{
    http().close();
}

int mantis::MantisApp::run() const {
    if (!m_router->initialize())
        quit(-1, "Failed to initialize router!");

    if (!m_http->listen(m_host, m_port))
        return -1;

    return 0;
}

mantis::DatabaseUnit& mantis::MantisApp::db() const
{
    return *m_database;
}

mantis::LoggingUnit& mantis::MantisApp::log() const
{
    return *m_logger;
}

mantis::HttpUnit& mantis::MantisApp::http() const
{
    return *m_http;
}

argparse::ArgumentParser& mantis::MantisApp::cmd() const
{
    return *m_opts;
}

mantis::Router& mantis::MantisApp::router() const
{
    return *m_router;
}

mantis::Validator& mantis::MantisApp::validators() const
{
    return *m_validators;
}

mantis::ExprEvaluator& mantis::MantisApp::evaluator() const
{
    return *m_exprEval;
}

void mantis::MantisApp::setDbType(const DbType& dbType)
{
    m_dbType = dbType;
}

std::string mantis::MantisApp::jwtSecretKey()
{
    // This is the default secret key, override it through environment variable
    // MANTIS_JWT_SECRET, recommended to override this key
    // TODO add commandline input for overriding the key
    return getEnvOrDefault("MANTIS_JWT_SECRET", "ed12086b9a609a5e410053b0541cb2d8da7087c1bb5e045962377d323ea6eb59");
}

mantis::DbType mantis::MantisApp::dbType() const
{
    return m_dbType;
}

int mantis::MantisApp::port() const
{
    return m_port;
}

void mantis::MantisApp::setPort(const int& port)
{
    if (port < 0 || port > 65535)
        return;

    m_port = port;
    Log::debug("Setting Server Port to {}", port);
}

std::string mantis::MantisApp::host() const
{
    return m_host;
}

void mantis::MantisApp::setHost(const std::string& host)
{
    if (host.empty())
        return;

    m_host = host;
    Log::debug("Setting Server Host to {}", host);
}

int mantis::MantisApp::poolSize() const
{
    return m_poolSize;
}

void mantis::MantisApp::setPoolSize(const int& pool_size)
{
    if (pool_size <= 0)
        return;

    m_poolSize = pool_size;
}

std::string mantis::MantisApp::publicDir() const
{
    return m_publicDir;
}

void mantis::MantisApp::setPublicDir(const std::string& dir)
{
    if (dir.empty())
        return;

    m_publicDir = dir;
}

std::string mantis::MantisApp::dataDir() const
{
    return m_dataDir;
}

void mantis::MantisApp::setDataDir(const std::string& dir)
{
    if (dir.empty())
        return;

    m_dataDir = dir;
}

inline bool mantis::MantisApp::ensureDirsAreCreated() const
{
    // Data Directory
    if (!createDirs(resolvePath(m_dataDir)))
        return false;

    if (!createDirs(resolvePath(m_publicDir)))
        return false;

    return true;
}
