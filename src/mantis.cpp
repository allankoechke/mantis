//
// Created by allan on 08/05/2025.
//

#include <mantis/mantis.h>
#include <filesystem>
#include <mantis/utils.h>

Mantis::MantisApp::MantisApp()
    : m_opts(std::make_shared<AnyOption>())
{
    // Enable Multi Sinks
    Logger::Config();

    // Set initial public directory
    auto dir = DirFromPath("./public");
    SetPublicDir(dir);

    // Set initial data directory
    dir = DirFromPath("./data");
    SetDataDir(dir);

    // Pass an instance of this ptr into the created Database instance
    m_dbMgr = std::make_shared<DatabaseMgr>(*this);
    m_svrMgr = std::make_shared<ServerMgr>(*this);
}

Mantis::MantisApp::~MantisApp()
{
    Logger::Shutdown();
}

int Mantis::MantisApp::ProcessCMD(const int argc, char* argv[])
{
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
        Logger::SetLogLevel(LogLevel::DEBUG);
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
        SetHost(host);
        Logger::Debug("Setting Server Host to [{}]", host);
    }

    if (m_opts->getValue('p') != nullptr || m_opts->getValue("port") != nullptr)
    {
        const auto port = std::atoi(m_opts->getValue("port"));
        SetPort(port);
        Logger::Debug("Setting Server Port to [{}]", port);
    }

    if (m_opts->getValue("publicDir") != nullptr)
    {
        const auto pth = m_opts->getValue("publicDir");
        const auto dir = DirFromPath(pth);
        SetPublicDir(dir);
    }

    if (m_opts->getValue("dataDir") != nullptr)
    {
        const auto pth = m_opts->getValue("dataDir");
        const auto dir = DirFromPath(pth);
        SetDataDir(dir);
    }

    if (m_opts->getValue('d') != nullptr || m_opts->getValue("db") != nullptr)
    {
        std::string db = m_opts->getValue("db");
        ToLowerCase(db);

        if (db == "sqlite")
        {
            m_dbMgr->SetDatabaseType(SQLITE);
        }

        if (db == "mysql")
        {
            m_dbMgr->SetDatabaseType(MYSQL);
        }

        if (db == "psql")
        {
            m_dbMgr->SetDatabaseType(PSQL);
        }

        else
        {
            Quit(-1, "Backend Database '" + db + "' is unknown!");
        }

        Logger::Debug("Setting Backend Database to [{}]", db);
    }

    if (m_opts->getFlag("serve"))
    {
        if (m_dbMgr->DbInit())
            return Start();

        Logger::Critical("Database was not opened");
        Quit(-1, "Database opening failed!");
        return 1;
    }

    // Initiate Db if it does not exist, yet!
    if (!m_dbMgr->DbInit())
        Quit(-1, "Database opening failed!");

    std::cout << std::endl;
    return 0;
}

int Mantis::MantisApp::Quit(const int& exitCode, const std::string& reason)
{
    // Do some cleanup if need be ...
    // m_db->CloseIfOpened();
    // m_svr->CloseIfOpened();

    if (exitCode != 0)
        Logger::Critical("Exiting Application with Code = {}", exitCode);
    else
        Logger::Info("Application exiting normally!");

    std::exit(exitCode);
}

int Mantis::MantisApp::Start()
{
    if (!EnsureDirsAreCreated())
        return -1;

    if (!m_dbMgr->EnsureDatabaseSchemaLoaded())
        return -1;

    Logger::Info("Starting listening on {}:{}", m_svrMgr->Host(), m_svrMgr->Port());
    return m_svrMgr->StartListening();
}

int Mantis::MantisApp::Start(const std::string& host, const int& port)
{
    if (port < 0 || port > 65535)
    {
        Logger::Critical("Invalid port number [{}]!", port);
        return false;
    }

    SetPort(port);
    SetHost(host);

    return Start();
}

int Mantis::MantisApp::Stop() const
{
    return m_svrMgr->StopListening();
}

std::shared_ptr<Mantis::ServerMgr> Mantis::MantisApp::GetSvrMgr() const
{
    return m_svrMgr;
}

std::shared_ptr<AnyOption> Mantis::MantisApp::GetCmdParser() const
{
    return m_opts;
}

std::shared_ptr<Mantis::DatabaseMgr> Mantis::MantisApp::GetDbMgr() const
{
    return m_dbMgr;
}

void Mantis::MantisApp::SetPort(const int& port) const
{
    if (port < 0 || port > 65535)
        return;

    m_svrMgr->SetPort(port);
}

void Mantis::MantisApp::SetHost(const std::string& host) const
{
    if (host.empty())
        return;

    m_svrMgr->SetHost(host);
}

void Mantis::MantisApp::SetPublicDir(const std::string& dir)
{
    if (dir.empty())
        return;
    m_publicDir = dir;
}

std::string Mantis::MantisApp::PublicDir() const
{
    return m_publicDir;
}

void Mantis::MantisApp::SetDataDir(const std::string& dir)
{
    if (dir.empty())
        return;

    m_dataDir = dir;
}

std::string Mantis::MantisApp::DataDir() const
{
    return m_dataDir;
}

fs::path Mantis::MantisApp::ResolvePath(const std::string& input_path)
{
    fs::path path(input_path);

    if (!path.is_absolute())
    {
        // Resolve relative to app binary
        path = fs::absolute(path);
    }

    return path;
}

bool Mantis::MantisApp::CreateDirs(const fs::path& path)
{
    try
    {
        if (!fs::exists(path))
        {
            fs::create_directories(path); // creates all missing parent directories too
            // std::cout << "Created directory: " << path << '\n';
        }
        else
        {
            // std::cout << "Directory already exists: " << path << '\n';
        }

        return true;
    }
    catch (const fs::filesystem_error& e)
    {
        Logger::Critical("Filesystem error while creating directory '{}', reason: {}",
            path.string(), e.what());
    }

    return false;
}

std::string Mantis::MantisApp::DirFromPath(const std::string& path)
{
    if (const auto dir = ResolvePath(path); CreateDirs(dir))
        return dir.string();

    return std::string("");
}

inline bool Mantis::MantisApp::EnsureDirsAreCreated() const
{
    // Data Directory
    if (!CreateDirs(ResolvePath(m_dataDir)))
        return false;

    if (!CreateDirs(ResolvePath(m_publicDir)))
        return false;

    return true;
}
