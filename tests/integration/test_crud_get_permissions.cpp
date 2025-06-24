//
// Created by allan on 19/06/2025.
//

#include "test_access_permissions_base.h"

// Get Rule Tests
TEST_F(AccessPermissionTest, GetRule_AllowsOwnerAccess) {
    // const std::string token = createUserAndGetToken("owner@test.com");
    //
    // // Create a record first
    // const nlohmann::json record = {{"title", "Test Record"}, {"user_id", "user123"}};
    // const httplib::Headers headers = {{"Authorization", "Bearer " + token}};
    //
    // auto create_result = client->Post("/api/v1/test_permissions", headers,
    //     record.dump(), "application/json");
    // // EXPECT_EQ(create_result->status, 201);
    //
    // auto create_response = nlohmann::json::parse(create_result->body);
    // const std::string record_id = create_response["data"]["id"];
    //
    // // Try to get the record - should succeed if auth.id matches
    // auto get_result = client->Get("/api/v1/test_permissions/" + record_id, headers);
    // EXPECT_EQ(get_result->status, 200);
}

TEST_F(AccessPermissionTest, GetRule_DeniesNonOwnerAccess) {
    // const std::string owner_token = createUserAndGetToken("owner@test.com");
    // const std::string other_token = createUserAndGetToken("other@test.com");
    //
    // // Create record with owner token
    // const nlohmann::json record = {{"title", "Private Record"}, {"user_id", "owner123"}};
    // httplib::Headers owner_headers = {{"Authorization", "Bearer " + owner_token}};
    //
    // auto create_result = client->Post("/api/v1/test_permissions", owner_headers,
    //     record.dump(), "application/json");
    // auto create_response = nlohmann::json::parse(create_result->body);
    // std::string record_id = create_response["data"]["id"];
    //
    // // Try to access with different user token
    // httplib::Headers other_headers = {{"Authorization", "Bearer " + other_token}};
    // auto get_result = client->Get("/api/v1/test_permissions/" + record_id, other_headers);

    // EXPECT_EQ(get_result->status, 403);
}
