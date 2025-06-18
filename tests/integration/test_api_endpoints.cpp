//
// Created by allan on 18/06/2025.
//

#include <gtest/gtest.h>
#include <httplib.h>
#include  <mantis/app/app.h>
#include "mantis/mantis.h"

class APITest : public ::testing::Test {
protected:
    void SetUp() override {
        app = std::make_unique<mantis::MantisApp>();
        // app->initialize();
        // app->parseArgs(0, nullptr); // Use defaults

        // Start server in background thread
        server_thread = std::thread([this]() {
            app->run();
        });

        // Wait for server to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        client = std::make_unique<httplib::Client>("http://localhost:7070");
    }

    void TearDown() override {
        app->close();
        if (server_thread.joinable()) {
            server_thread.join();
        }
    }

    std::unique_ptr<mantis::MantisApp> app;
    std::unique_ptr<httplib::Client> client;
    std::thread server_thread;
};

TEST_F(APITest, GetAllRecordsFromTable) {
    // Create a test table first through system API
    auto result = client->Get("/api/v1/test_table");
    EXPECT_EQ(result->status, 200);

    auto json_response = nlohmann::json::parse(result->body);
    EXPECT_EQ(json_response["status"], 200);
}

TEST_F(APITest, CreateAndRetrieveRecord) {
    nlohmann::json record = {{"name", "Test User"}, {"email", "test@example.com"}};

    auto create_result = client->Post("/api/v1/users",
        record.dump(), "application/json");
    EXPECT_EQ(create_result->status, 201);

    auto response = nlohmann::json::parse(create_result->body);
    std::string record_id = response["data"]["id"];

    auto get_result = client->Get("/api/v1/users/" + record_id);
    EXPECT_EQ(get_result->status, 200);
}
