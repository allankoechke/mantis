//
// Created by allan on 18/06/2025.
//

#include <gtest/gtest.h>
#include <httplib.h>
#include "mantis/app/app.h"

class AuthTest : public ::testing::Test {
    protected:
    void SetUp() override {
        // Just create the HTTP client - server is already running
        client = std::make_unique<httplib::Client>("http://localhost:8080");

        // Clean up any test data from previous tests
        cleanupTestData();

        // Create test tables for this test suite
        createTestTableWithRules();
    }

    void TearDown() override {
        // Clean up test data after each test
        cleanupTestData();
    }

    void cleanupTestData() const
    {
        // Delete test tables and data created by this test
        client->Delete("/api/v1/tables/test_permissions");
        client->Delete("/api/v1/tables/admin_only");
        // etc.
    }

    void createTestTableWithRules() const
    {
        // Create a table with specific access rules for testing
        const nlohmann::json table_schema = {
            {"name", "test_permissions"},
            {"type", "base"},
            {"listRule", "auth.table == 'users'"},           // Only users can list
            {"getRule", "auth.id == req.id"},                // Users can only get their own records
            {"addRule", "auth.table == 'users'"},            // Only users can add
            {"updateRule", "auth.id == req.id"},             // Users can only update their own records
            {"deleteRule", "auth.table == '__admin'"},       // Only admins can delete
            {"fields", nlohmann::json::array({
                {{"name", "title"}, {"type", "string"}, {"required", true}},
                {{"name", "user_id"}, {"type", "string"}, {"required", true}}
            })}
        };

        // Create table through system API
        auto result = client->Post("/api/v1/tables",
            table_schema.dump(), "application/json");
    }

    [[nodiscard]]
    std::string createUserAndGetToken(const std::string& email) const
    {
        // Create user
        const nlohmann::json user = {
            {"name", "Test User"},
            {"email", email},
            {"password", "testpass123"}
        };

        client->Post("/api/v1/users", user.dump(), "application/json");

        // Login and get token
        const nlohmann::json login = {{"email", email}, {"password", "testpass123"}};
        auto auth_result = client->Post("/api/v1/users/auth-with-password",
            login.dump(), "application/json");

        auto response = nlohmann::json::parse(auth_result->body);
        return response["data"]["token"].get<std::string>();
    }

    [[nodiscard]]
    std::string createAdminAndGetToken() const
    {
        // Create admin user
        const nlohmann::json admin = {
            {"name", "Admin User"},
            {"email", "admin@test.com"},
            {"password", "adminpass123"}
        };

        client->Post("/api/v1/admins", admin.dump(), "application/json");

        // Login as admin
        const nlohmann::json login = {{"email", "admin@test.com"}, {"password", "adminpass123"}};
        auto auth_result = client->Post("/api/v1/admins/auth-with-password",
            login.dump(), "application/json");

        auto response = nlohmann::json::parse(auth_result->body);
        return response["data"]["token"].get<std::string>();
    }

    std::unique_ptr<httplib::Client> client;
};

TEST_F(AuthTest, LoginWithValidCredentials) {
    // First create a user
    // const nlohmann::json user = {
    //     {"name", "Test User"},
    //     {"email", "test@example.com"},
    //     {"password", "testpass123"}
    // };
    //
    // auto create_result = client->Post("/api/v1/auth_users",
    //     user.dump(), "application/json");
    // // EXPECT_EQ(create_result->status, 201);
    //
    // // Then authenticate
    // nlohmann::json login = {
    //     {"email", "test@example.com"},
    //     {"password", "testpass123"}
    // };
    //
    // auto auth_result = client->Post("/api/v1/auth_users/auth-with-password",
    //     login.dump(), "application/json");
    // // EXPECT_EQ(auth_result->status, 200);
    //
    // auto response = nlohmann::json::parse(auth_result->body);
    // EXPECT_FALSE(response["data"]["token"].get<std::string>().empty());
}

TEST_F(AuthTest, AccessProtectedEndpointWithToken) {
    // Get token from login
    // const std::string token = createUserAndGetToken("protected@user.com");
    //
    // const httplib::Headers headers = {
    //     {"Authorization", "Bearer " + token}
    // };
    //
    // auto result = client->Get("/api/v1/protected_table", headers);
    // EXPECT_EQ(result->status, 200);
}