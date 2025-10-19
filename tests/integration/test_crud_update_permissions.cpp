//
// Created by allan on 18/06/2025.
//
#include <catch2/catch_all.hpp>

// Update Rule Tests
TEST_CASE("AccessPermissionTest, UpdateRule_AllowsOwnerUpdates", "[integration]")
{
    // const std::string token = createUserAndGetToken("updater@test.com");
    // httplib::Headers headers = {{"Authorization", "Bearer " + token}};
    //
    // // Create record
    // const nlohmann::json record = {{"title", "Original Title"}, {"user_id", "updater123"}};
    // auto create_result = client->Post("/api/v1/test_permissions", headers ,
    //     record.dump(), "application/json");
    //
    // auto create_response = nlohmann::json::parse(create_result->body);
    // const std::string record_id = create_response["data"]["id"];
    //
    // // Update record
    // nlohmann::json update = {{"title", "Updated Title"}};
    // auto update_result = client->Patch("/api/v1/test_permissions/" + record_id, headers ,
    //     update.dump(), "application/json");

    // EXPECT_EQ(update_result->status, 200);
}

TEST_CASE("AccessPermissionTest, UpdateRule_DeniesNonOwnerUpdates", "[integration]")
{
    // std::string owner_token = createUserAndGetToken("owner@test.com");
    // std::string other_token = createUserAndGetToken("other@test.com");
    //
    // // Create record with owner
    // nlohmann::json record = {{"title", "Owner's Record"}, {"user_id", "owner123"}};
    // httplib::Headers owner_headers = {{"Authorization", "Bearer " + owner_token}};
    //
    // auto create_result = client->Post("/api/v1/test_permissions", owner_headers ,
    //     record.dump(), "application/json");
    // auto create_response = nlohmann::json::parse(create_result->body);
    // std::string record_id = create_response["data"]["id"];
    //
    // // Try to update with different user
    // nlohmann::json update = {{"title", "Hacked Title"}};
    // httplib::Headers other_headers = {{"Authorization", "Bearer " + other_token}};
    //
    // auto update_result = client->Patch("/api/v1/test_permissions/" + record_id, other_headers,
    //     update.dump(), "application/json");

    // EXPECT_EQ(update_result->status, 403);
}
