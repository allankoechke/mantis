//
// Created by allan on 18/06/2025.
//

#include <catch2/catch_all.hpp>
#include <httplib.h>
#include "mantis/app/app.h"

TEST_CASE("AuthTest, LoginWithValidCredentials", "[integration]") {
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

TEST_CASE("AuthTest, AccessProtectedEndpointWithToken", "[integration]") {
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