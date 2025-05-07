//
// Created by allan on 08/05/2025.
//

#include <iostream>
#include <memory>
#include <anyoption.h>
// #include <mantis.h>

int main(int argc, char* argv[])
{
    // Let's create an instance of 'AnyOption' to provide
    // a commandline args utility
    const auto opt = make_shared<AnyOption>();

    opt->setVerbose(); /* print warnings about unknown options */

    // SET THE USAGE/HELP
    opt->addUsage("usage: ");
    opt->addUsage("mantis [Optional Flags] --serve ");
    opt->addUsage("");
    opt->addUsage(" -h  --help  		        Prints this help ");
    opt->addUsage(" -p  --port <port>       Server Port (default: 7070)");
    opt->addUsage(" -h  --host <host>       Server Host (default: 0.0.0.0) ");
    opt->addUsage(" --publicDir   <dir>     Static files directory (default: ./public) ");
    opt->addUsage(" --dataDir     <dir>     Data directory (default: ./data) ");
    opt->addUsage(" --serve                 Start & Run the HTTP Server ");
    opt->addUsage("");

    opt->setFlag("help", 'h');
    opt->setOption("host", 'i');
    opt->setOption("port", 'p');
    opt->setOption("publicDir");
    opt->setOption("dataDir");
    opt->setCommandFlag("serve");

    /* go through the command line and get the options  */
    opt->processCommandArgs(argc, argv);

    if (!opt->hasOptions())
    {
        /* print usage if no options */
        opt->printUsage();
    }

    // GET THE VALUES
    if (opt->getFlag("help") || opt->getFlag('h'))
        opt->printUsage();

    if (opt->getValue('h') != nullptr || opt->getValue("host") != nullptr)
        std::cout << "Host: " << opt->getValue("host") << std::endl;

    if (opt->getValue('p') != nullptr || opt->getValue("port") != nullptr)
        std::cout << "Port: " << opt->getValue("port") << std::endl;

    if (opt->getValue("publicDir") != nullptr)
        std::cout << "PublicDir: " << opt->getValue("publicDir") << std::endl;

    if (opt->getValue("dataDir") != nullptr)
        std::cout << "DataDir: " << opt->getValue("dataDir") << std::endl;

    if (opt->getFlag("serve"))
        std::cout << "Start the HTTP Server " << std::endl;

    std::cout << std::endl;
}
