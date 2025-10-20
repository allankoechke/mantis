//
// Created by allan on 18/10/2025.
//

#ifndef MANTISAPP_TEST_FIXTURE_H
#define MANTISAPP_TEST_FIXTURE_H

// #define CATCH_CONFIG_NOSTDOUT
// #include <catch2/catch_test_macros.hpp>

#include <thread>
#include <chrono>
#include <httplib.h>
#include <iostream>
#include <filesystem>

#include "mantis/app/app.h"
#include "mantis/core/http.h"
#include "mantis/utils/utils.h"

namespace fs = std::filesystem;

struct TestFixture
{
    int port = 7075;
    mantis::MantisApp& mApp;

private:
    TestFixture(const int argc, char* argv[])
        : mApp(mantis::MantisApp::create(argc, argv))
    {
        std::cout << "[TestFixture] Setting up DB and starting server...\n";
    }

public:
    static TestFixture& instance(const int argc, char* argv[])
    {
        static TestFixture _instance{argc, argv};
        return _instance;
    }

    mantis::MantisApp& mantisApp() const
    {
        return mantis::MantisApp::instance();
    }

    // Wait until the server port responds
    bool waitForServer(const int retries = 50, const int delayMs = 500) const
    {
        httplib::Client cli("http://localhost", port);
        for (int i = 0; i < retries; ++i)
        {
            if (auto res = cli.Get("/api/v1/health"); res && res->status == 200)
                return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        }
        return false;
    }

    void teardownOnce(const fs::path& base_path) const
    {
        std::cout << "[TestFixture] Shutting down server...\n";

        // Graceful shutdown
        mantisApp().close();

        // Cleanup the temporary directory & files
        try
        {
            std::filesystem::remove_all(base_path);
            std::cout << "[TestFixture] Removing directory " << base_path.string() << "...\n";
        }
        catch (const std::exception& e)
        {
            std::cerr << "[TestFixture] Error removing file: " << e.what() << "\n";
        }

        std::cout << "[TestFixture] Server stopped.\n";
    }

    static httplib::Client client()
    {
        return httplib::Client("http://localhost", 7075);
    }
};

#endif //MANTISAPP_TEST_FIXTURE_H
