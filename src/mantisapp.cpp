/**
 * @file mantisapp.cpp
 * @brief Standalone `mantisapp` executable entrypoint
 *
 * Created by allan on 08/05/2025.
 */

#include "../include/mantis/app/app.h"

/**
 * @brief MantisApp standalone entrypoint
 * @param argc Argument count
 * @param argv Argument list
 * @return Non-zero if the application did not exit correctly
 */
int main(const int argc, char* argv[])
{
    // Create `MantisApp` instance with the passed in arguments
    auto& app = mantis::MantisApp::create(argc, argv);

    // Run the http server listening loop
    return app.run();
}
