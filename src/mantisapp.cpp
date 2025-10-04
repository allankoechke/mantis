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
    mantis::MantisApp app(argc, argv);

    // Initialize the app, this ensures all `Mantis` units are
    // initialized and running.
    app.init();

    // Run the http server listening loop
    return app.run();

    // Alternatively, you can just do
    // return initAndRun();
}
