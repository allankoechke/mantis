/**
 * @file mantisapp.cpp
 * @brief Standalone `mantisapp` executable entrypoint
 *
 * Created by allan on 08/05/2025.
 */

#include "../include/mantis/mantis.h"

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

    // Or simply
    // Create the JSON object
    // {
    //     "dev": nullptr/true/etc, // the value here does not matter
    //     "serve": {
    //         "port": 9089
    //     }
    // }
    // const json args{{"dev", nullptr}, {"serve", {{"port", 9089}}}};
    // auto& app = mantis::MantisApp::create(args);

    // Run the http server listening loop
    return app.run();
}