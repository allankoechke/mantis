//
// Created by allan on 18/06/2025.
//

#include <catch2/catch_all.hpp>
#include <httplib.h>
#include  <mantis/app/app.h>

TEST_CASE("APITest, GetAllRecordsFromTable", "[integration]") {
    // Create a test table first through system API
    // auto result = client->Get("/api/v1/test_table");
    // EXPECT_EQ(result->status, 200);

    // auto json_response = nlohmann::json::parse(result->body);
    //EXPECT_EQ(json_response["status"], 200);
}

TEST_CASE("APITest, CreateAndRetrieveRecord", "[integration]") {
    // nlohmann::json record;
    // record["name"] = "Test User";
    // record["email"] = "createandretrieve@record.com";
    //
    // auto create_result = client->Post("/api/v1/users",
    //     record.dump(), "application/json");
    // EXPECT_EQ(create_result->status, 201);

    // auto response = nlohmann::json::parse(create_result->body);
    // const std::string record_id = response["data"]["id"];
    //
    // auto get_result = client->Get("/api/v1/users/" + record_id);
    //EXPECT_EQ(get_result->status, 200);
}
