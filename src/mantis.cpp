//
// Created by allan on 08/05/2025.
//

#include <mantis/mantis.h>
#include <filesystem>

MantisApp::MantisApp()
    :   m_svr(std::make_shared<httplib::Server>()),
        m_opts(std::make_shared<AnyOption>()),
        m_port(7070),
        m_host("127.0.0.1"),
        m_publicDir("./public"),
        m_dataDir("./data")
{}

int MantisApp::ProcessCMD(int argc, char *argv[])
{
    m_opts->setVerbose(); /* print warnings about unknown options */

    // SET THE USAGE/HELP
    m_opts->addUsage("usage: ");
    m_opts->addUsage("mantis [Optional Flags] --serve ");
    m_opts->addUsage("");
    m_opts->addUsage(" -h  --help  		        Prints this help ");
    m_opts->addUsage(" -p  --port <port>       Server Port (default: 7070)");
    m_opts->addUsage(" -h  --host <host>       Server Host (default: 0.0.0.0) ");
    m_opts->addUsage(" --publicDir   <dir>     Static files directory (default: ./public) ");
    m_opts->addUsage(" --dataDir     <dir>     Data directory (default: ./data) ");
    m_opts->addUsage(" --serve                 Start & Run the HTTP Server ");
    m_opts->addUsage("");

    m_opts->setFlag("help", 'h');
    m_opts->setOption("host", 'i');
    m_opts->setOption("port", 'p');
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
        std::cout << "Setting Server Host to [" << host << "]" << std::endl;
        m_host = host;
    }

    if (m_opts->getValue('p') != nullptr || m_opts->getValue("port") != nullptr)
    {
        const auto port = std::atoi(m_opts->getValue("port"));
        std::cout << "Setting Server Port to [" << port << "]" << std::endl;
        m_port = port;
    }

    if (m_opts->getValue("publicDir") != nullptr)
    {
        std::cout << "PublicDir: " << m_opts->getValue("publicDir") << std::endl;
    }

    if (m_opts->getValue("dataDir") != nullptr)
    {
        std::cout << "DataDir: " << m_opts->getValue("dataDir") << std::endl;
    }

    if (m_opts->getFlag("serve"))
    {
        std::cout << "Start the HTTP Server " << std::endl;
        return Start();
    }

    std::cout << std::endl;

    return 0;
}

int MantisApp::Start()
{
    if (!EnsureDirsAreCreated())
        return -1;

    if (!m_db->EnsureDatabaseSchemaLoaded())
        return -1;

    std::cout << "Starting listening on " << m_host << ":" << m_port << std::endl;
    return m_svr->listen(m_host, m_port);
}

int MantisApp::Start(const std::string& host, const int& port)
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

int MantisApp::Stop() const
{
    if (m_svr->is_running())
        m_svr->stop();

    return 0;
}

std::shared_ptr<httplib::Server> MantisApp::Server() const
{
    return m_svr;
}

std::shared_ptr<AnyOption> MantisApp::CmdParser() const
{
    return m_opts;
}

void MantisApp::SetPort(const int& port)
{
    if (port < 0 || port > 65535)
        return;

    m_port = port;
}

void MantisApp::SetHost(const std::string& host)
{
    if (host.empty())
        return;

    m_host = host;
}

void MantisApp::SetPublicDir(const std::string& dir)
{
    if (dir.empty())
        return;
    m_publicDir = dir;
}

void MantisApp::SetDataDir(const std::string& dir)
{
    if (dir.empty())
        return;

    m_dataDir = dir;
}

fs::path MantisApp::ResolvePath(const std::string& input_path)
{
    fs::path path(input_path);

    if (!path.is_absolute()) {
        // Resolve relative to app binary
        path = fs::absolute(path);
    }

    return path;
}

bool MantisApp::CreateDirs(const fs::path& path)
{
    try {
        if (!fs::exists(path)) {
            fs::create_directories(path); // creates all missing parent directories too
            std::cout << "Created directory: " << path << '\n';
        } else {
            std::cout << "Directory already exists: " << path << '\n';
        }

        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    }

    return false;
}

inline bool MantisApp::EnsureDirsAreCreated() const
{
    // Data Directory
    if (!CreateDirs(ResolvePath(m_dataDir)))
        return false;

    if (!CreateDirs(ResolvePath(m_publicDir)))
        return false;

    return true;
}