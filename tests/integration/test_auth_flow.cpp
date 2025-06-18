//
// Created by allan on 18/06/2025.
//

#include <gtest/gtest.h>
#include <httplib.h>
#include "mantis/app/app.h"
#include <mantis/mantis.h>

class AuthTest : public ::testing::Test {
    // Similar setup to APITest
};

TEST_F(AuthTest, LoginWithValidCredentials) {
    // First create a user
    nlohmann::json user = {
        {"name", "Test User"},
        {"email", "test@example.com"},
        {"password", "testpass123"}
    };

    auto create_result = client->Post("/api/v1/auth_users",
        user.dump(), "application/json");
    EXPECT_EQ(create_result->status, 201);

    // Then authenticate
    nlohmann::json login = {
        {"email", "test@example.com"},
        {"password", "testpass123"}
    };

    auto auth_result = client->Post("/api/v1/auth_users/auth-with-password",
        login.dump(), "application/json");
    EXPECT_EQ(auth_result->status, 200);

    auto response = nlohmann::json::parse(auth_result->body);
    EXPECT_FALSE(response["data"]["token"].get<std::string>().empty());
}

TEST_F(AuthTest, AccessProtectedEndpointWithToken) {
    // Get token from login
    std::string token = getAuthToken();

    httplib::Headers headers = {
        {"Authorization", "Bearer " + token}
    };

    auto result = client->Get("/api/v1/protected_table", headers);
    EXPECT_EQ(result->status, 200);
}