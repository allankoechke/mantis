//
// Created by allan on 19/06/2025.
//

#ifndef TEST_ENVIRONMENT_H
#define TEST_ENVIRONMENT_H

#include <httplib.h>
#include <gtest/gtest.h>
#include <mantis/app/app.h>
#include <mantis/core/models/models.h>
#include <mantis/core/database.h>
#include <mantis/utils/utils.h>

#include "test_helpers.h"

class MantisTestEnvironment final : public ::testing::Environment {
public:
    void SetUp() override {
        const auto dataPath = TestDatabase::getTestBasePath() / "data";
        const auto wwwPath = TestDatabase::getTestBasePath() / "www";

        // Create test arguments for the global server
        const char* test_args[] = {
            "mantis_test",
            "--database", "SQLITE",
            "--dataDir", dataPath.string().c_str(),
            "--publicDir", wwwPath.string().c_str(),
            "--dev",
            "serve",
            "--port", "8081",
        };
        int argc = std::size(test_args);

        // Create and start the global Mantis app
        global_app = std::make_unique<mantis::MantisApp>(argc, const_cast<char**>(test_args));
        global_app->init(); // Initialize App

        // Start server in background thread
        server_thread = std::thread([this]() {
            [[maybe_unused]] auto ok = global_app->run();
        });

        // Wait for server to be ready
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Verify server is running
        httplib::Client test_client("http://localhost:8081");
        if (auto result = test_client.Get("/"); !result || result->status != 404) { // 404 is expected for root path
            throw std::runtime_error("Failed to start test server");
        }

        std::cout << "Base Path: " << TestDatabase::getTestBasePath() << std::endl;
    }

    void TearDown() override {
        if (global_app) {
            global_app->close();
        }
        if (server_thread.joinable()) {
            server_thread.join();
        }

        mantis::MantisApp::instance().db().disconnect();
        TestDatabase::cleanupTestDb();
    }

    static mantis::MantisApp& GetApp() { return *global_app; }

private:
    static std::unique_ptr<mantis::MantisApp> global_app;
    static std::thread server_thread;
};

// Static member definitions
std::unique_ptr<mantis::MantisApp> MantisTestEnvironment::global_app = nullptr;
std::thread MantisTestEnvironment::server_thread;

#endif //TEST_ENVIRONMENT_H
