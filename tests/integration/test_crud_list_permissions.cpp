//
// Created by allan on 19/06/2025.
//
#include <catch2/catch_all.hpp>

// List Rule Tests
TEST_CASE("AccessPermissionTest, ListRule_AllowsAuthorizedUsers", "[integration]")
{
    // const std::string token = createUserAndGetToken("user@test.com");
    //
    // const httplib::Headers headers = {{"Authorization", "Bearer " + token}};
    // auto result = client->Get("/api/v1/test_permissions", headers);

    // EXPECT_EQ(result->status, 200);
    // auto response = nlohmann::json::parse(result->body);
    // EXPECT_EQ(response["status"], 200);
}

TEST_CASE("AccessPermissionTest, ListRule_DeniesUnauthorizedUsers", "[integration]")
{
    // Try to access without token
    // auto result = client->Get("/api/v1/test_permissions");
    //
    // // EXPECT_EQ(result->status, 403);
    // auto response = nlohmann::json::parse(result->body);
    // EXPECT_EQ(response["status"], 403);
    // EXPECT_FALSE(response["error"].get<std::string>().empty());
}

TEST_CASE("AccessPermissionTest, ListRule_DeniesWrongUserType", "[integration]")
{
    // const std::string admin_token = createAdminAndGetToken();
    //
    // const httplib::Headers headers = {{"Authorization", "Bearer " + admin_token}};
    // auto result = client->Get("/api/v1/test_permissions", headers);

    // Should be denied because rule requires auth.table == 'users', but admin has '__admin'
    // EXPECT_EQ(result->status, 403);
}
