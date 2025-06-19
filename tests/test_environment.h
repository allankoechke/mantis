//
// Created by allan on 19/06/2025.
//

#ifndef TEST_ENVIRONMENT_H
#define TEST_ENVIRONMENT_H

#include <gtest/gtest.h>
#include "mantis/app/app.h"
#include <httplib.h>

class MantisTestEnvironment final : public ::testing::Environment {
public:
    void SetUp() override {
        // Create test arguments for the global server
        const char* test_args[] = {
            "mantis_test",
            "--database", "SQLITE",
            "--dataDir", "./test_data",
            "--dev",
            "serve",
            "--port", "8081",
        };
        int argc = std::size(test_args);

        // Create and start the global Mantis app
        global_app = std::make_unique<mantis::MantisApp>(argc, const_cast<char**>(test_args));

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
    }

    void TearDown() override {
        if (global_app) {
            global_app->close();
        }
        if (server_thread.joinable()) {
            server_thread.join();
        }
    }

    static mantis::MantisApp* GetApp() { return global_app.get(); }

private:
    static std::unique_ptr<mantis::MantisApp> global_app;
    static std::thread server_thread;
};

// Static member definitions
std::unique_ptr<mantis::MantisApp> MantisTestEnvironment::global_app = nullptr;
std::thread MantisTestEnvironment::server_thread;

#endif //TEST_ENVIRONMENT_H
