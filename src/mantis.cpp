//
// Created by allan on 08/05/2025.
//

#include <mantis/mantis.h>
#include <filesystem>
#include <mantis/utils.h>

Mantis::MantisApp::MantisApp()
    : m_svr(std::make_shared<httplib::Server>()),
      m_opts(std::make_shared<AnyOption>()),
      m_port(7070),
      m_host("127.0.0.1")
{
    // Set initial public directory
    auto dir = DirFromPath("./public");
    SetPublicDir(dir);

    // Set initial data directory
    dir = DirFromPath("./data");
    SetDataDir(dir);

    // Pass an instance of this ptr into the created Database instance
    m_dbMgr = std::make_shared<Mantis::DatabaseMgr>(*this);
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
    m_opts->addUsage("");

    m_opts->setFlag("help", 'h');
    m_opts->setOption("host", 'i');
    m_opts->setOption("port", 'p');
    m_opts->setOption("db", 'd');
    m_opts->setOption("publicDir");
    m_opts->setOption("dataDir");
    m_opts->setCommandFlag("serve");

    /* go through the command line and get the options  */
    m_opts->processCommandArgs(argc, argv);

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
        m_host = host;
        std::cout << "Setting Server Host to [" << host << "]" << std::endl;
    }

    if (m_opts->getValue('p') != nullptr || m_opts->getValue("port") != nullptr)
    {
        const auto port = std::atoi(m_opts->getValue("port"));
        m_port = port;
        std::cout << "Setting Server Port to [" << port << "]" << std::endl;
    }

    if (m_opts->getValue("publicDir") != nullptr)
    {
        const auto pth = m_opts->getValue("publicDir");
        const auto dir = DirFromPath(pth);
        SetPublicDir(dir);
        std::cout << "PublicDir: " << m_publicDir << std::endl;
    }

    if (m_opts->getValue("dataDir") != nullptr)
    {
        const auto pth = m_opts->getValue("dataDir");
        const auto dir = DirFromPath(pth);
        SetDataDir(dir);
        std::cout << "DataDir: " << m_dataDir << std::endl;
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

        std::cout << "Setting Backend Database to [" << db << "]" << std::endl;
    }

    if (m_opts->getFlag("serve"))
    {
        std::cout << "Start the HTTP Server " << std::endl;
        if (m_dbMgr->DbInit())
            return Start();

        std::cerr << "Database was not opened" << std::endl;
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
        std::cerr << "Exiting, [" << exitCode << "] :  " << reason << std::endl;
    else
        std::cout << "Application exiting" << std::endl;

    std::exit(exitCode);
}

int Mantis::MantisApp::Start()
{
    if (!EnsureDirsAreCreated())
        return -1;

    if (!m_dbMgr->EnsureDatabaseSchemaLoaded())
        return -1;

    std::cout << "Starting listening on " << m_host << ":" << m_port << std::endl;
    return m_svr->listen(m_host, m_port);
}

int Mantis::MantisApp::Start(const std::string& host, const int& port)
{
    if (port < 0 || port > 65535)
    {
        std::cerr << "Invalid port number: " << port << std::endl;
        return false;
    }

    m_port = port;
    m_host = host;

    return Start();
}

int Mantis::MantisApp::Stop() const
{
    if (m_svr->is_running())
        m_svr->stop();

    return 0;
}

std::shared_ptr<httplib::Server> Mantis::MantisApp::Server() const
{
    return m_svr;
}

std::shared_ptr<AnyOption> Mantis::MantisApp::CmdParser() const
{
    return m_opts;
}

void Mantis::MantisApp::SetPort(const int& port)
{
    if (port < 0 || port > 65535)
        return;

    m_port = port;
}

void Mantis::MantisApp::SetHost(const std::string& host)
{
    if (host.empty())
        return;

    m_host = host;
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
            std::cout << "Created directory: " << path << '\n';
        }
        else
        {
            std::cout << "Directory already exists: " << path << '\n';
        }

        return true;
    }
    catch (const fs::filesystem_error& e)
    {
        std::cerr << "Filesystem error: " << e.what() << '\n';
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
