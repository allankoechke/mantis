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
#include "mantis/core/http.h"

class MantisTestEnvironment final : public ::testing::Environment {
public:
    void SetUp() override {
        const auto dataPath = TestDatabase::getTestBasePath() / "data";
        const auto wwwPath = TestDatabase::getTestBasePath() / "www";

        // Create test arguments for the global server
        m_testArgs = {
            "mantis_test", "--database", "SQLITE",
            "--dataDir", dataPath.string(),
            "--publicDir", wwwPath.string(),
            "--dev",
            "serve", "--port", "8081", "--host", "127.0.0.1"
        };

        std::vector<std::string> args_copy = m_testArgs;
        // Start server in background thread
        server_thread = std::thread([args_copy = std::move(args_copy)]() mutable -> void {
            char *argv[13];
            for (auto i = 0; i < args_copy.size(); ++i) {
                argv[i] = const_cast<char *>(args_copy.at(i).c_str());
            }

            // Create and start the global Mantis app
            const auto app = std::make_unique<mantis::MantisApp>(args_copy.size(), argv);
            app->init(); // Initialize App
            const auto _ = app->run();
            std::cout << "App Done Running!" << std::endl;
        });

        // Wait for server to be ready
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        if (mantis::MantisApp::instance().http().server().is_running()) {
            throw std::runtime_error("Failed to start test server");
        }
        // Verify server is running
        httplib::Client test_client("http://localhost:8081");
        if (auto result = test_client.Get("/"); !result || result->status != 404) {
            // 404 is expected for root path
            throw std::runtime_error("Failed to start test server");
        }
    }

    void TearDown() override {
        mantis::MantisApp::quit(0, "");

        if (server_thread.joinable()) {
            server_thread.join();
        }

        // mantis::MantisApp::instance().db().disconnect();
        // TestDatabase::cleanupTestDb();
        std::cout << std::endl << "Test Server Stopped" << std::endl;
    }

    static mantis::MantisApp &GetApp() { return mantis::MantisApp::instance(); }

private:
    // static std::shared_ptr<mantis::MantisApp> global_app;
    static std::thread server_thread;
    std::vector<std::string> m_testArgs;
};

// Static member definitions
// std::shared_ptr<mantis::MantisApp> MantisTestEnvironment::global_app = nullptr;
std::thread MantisTestEnvironment::server_thread;

#endif //TEST_ENVIRONMENT_H
