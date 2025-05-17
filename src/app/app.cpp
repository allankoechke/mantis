//
// Created by allan on 16/05/2025.
//

#include "../../include/mantis/mantis.h"

mantis::MantisApp::MantisApp(int argc, char** argv) {
    // Enable Multi Sinks
   Log::init();

    // Set initial public directory
    auto dir = dirFromPath("./public");
    setPublicDir(dir);

    // Set initial data directory
    dir = dirFromPath("./data");
    setDataDir(dir);

    parseArgs(argc, argv);
    initialize();
}

mantis::MantisApp::~MantisApp()
{
    Log::close();
}

void mantis::MantisApp::parseArgs(const int argc, char** argv) {
    m_opts->setVerbose(); /* print warnings about unknown options */

    // SET THE USAGE/HELP
    m_opts->addUsage("Usage: ");
    m_opts->addUsage("  mantis [Optional Flags] --serve ");
    m_opts->addUsage("  mantis -p 7070 --publicDir ./www --serve ");
    m_opts->addUsage("  mantis -p 7070 --db SQLITE --serve ");
    m_opts->addUsage("");
    m_opts->addUsage("");
    m_opts->addUsage("  -h  --help  		    Prints this help ");
    m_opts->addUsage("  -p  --port  <port>      Server Port (default: 7070)");
    m_opts->addUsage("  -h  --host  <host>      Server Host (default: 0.0.0.0) ");
    m_opts->addUsage("  -d  --db    <db>        Database type ['SQLITE', 'PSQL', 'MYSQL'] (default: SQLITE) ");
    m_opts->addUsage("  --publicDir   <dir>     Static files directory (default: ./public) ");
    m_opts->addUsage("  --dataDir     <dir>     Data directory (default: ./data) ");
    m_opts->addUsage("  --serve                 Start & Run the HTTP Server ");
    m_opts->addUsage("  --dev                   Print developer logs & SQL Statements ");
    m_opts->addUsage("");

    m_opts->setFlag("help", 'h');
    m_opts->setOption("host", 'i');
    m_opts->setOption("port", 'p');
    m_opts->setOption("db", 'd');
    m_opts->setOption("publicDir");
    m_opts->setOption("dataDir");
    m_opts->setCommandFlag("serve");
    m_opts->setCommandFlag("dev");

    /* go through the command line and get the options  */
    m_opts->processCommandArgs(argc, argv);

    if (m_opts->getFlag("dev"))
    {
        // Print developer messages ...
        // Set it to debug for now
        Log::setLogLevel(LogLevel::DEBUG);
    }

    if (!m_opts->hasOptions())
    {
        /* print usage if no options */
        m_opts->printUsage();
    }

    // GET THE VALUES
    if (m_opts->getFlag("help") || m_opts->getFlag('h'))
    {
        m_opts->printUsage();
    }

    if (m_opts->getValue('h') != nullptr || m_opts->getValue("host") != nullptr)
    {
        const auto host = m_opts->getValue("host");
        setHost(host);
    }

    if (m_opts->getValue('p') != nullptr || m_opts->getValue("port") != nullptr)
    {
        const auto port = std::atoi(m_opts->getValue("port"));
        setPort(port);
    }

    if (m_opts->getValue("publicDir") != nullptr)
    {
        const auto pth = m_opts->getValue("publicDir");
        const auto dir = dirFromPath(pth);
        setPublicDir(dir);
    }

    if (m_opts->getValue("dataDir") != nullptr)
    {
        const auto pth = m_opts->getValue("dataDir");
        const auto dir = dirFromPath(pth);
        setDataDir(dir);
    }

    if (m_opts->getValue('d') != nullptr || m_opts->getValue("db") != nullptr)
    {
        std::string db = m_opts->getValue("db");
        toLowerCase(db);

        if (db == "sqlite")
        {
            setDbType(SQLITE);
        }

        if (db == "mysql")
        {
            setDbType(MYSQL);
        }

        if (db == "psql")
        {
            setDbType(PSQL);
        }

        else
        {
            quit(-1, "Backend Database '" + db + "' is unknown!");
        }

        Log::debug("Setting Backend Database to [{}]", db);
    }

    // TODO ...
    if (m_opts->getFlag("serve"))
    {
        Log::critical("Database was not opened");
        quit(-1, "Database opening failed!");
    }

    std::cout << std::endl;
    // return 0;
}

void mantis::MantisApp::initialize() {
    m_logger = std::make_unique<LoggingUnit>();
    m_database = std::make_unique<DatabaseUnit>(this);
    m_http = std::make_unique<HttpUnit>();
    m_opts = std::make_unique<AnyOption>();

    // Directories ...
    if (!ensureDirsAreCreated())
        quit(-1, "Failed to create database directories!");

    // m_database->connect(db_backend_, db_conn_str_);
    // m_database->migrate();
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

AnyOption& mantis::MantisApp::cmd() const
{
    return *m_opts;
}

void mantis::MantisApp::setDbType(const DbType& dbType)
{
    m_dbType = dbType;
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
