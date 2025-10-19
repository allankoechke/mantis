//
// Created by allan on 18/06/2025.
//
#include <catch2/catch_all.hpp>
#include "mantis/core/jwt.h"
#include <nlohmann/json.hpp>

TEST_CASE("JWTTest, CreateValidToken", "[unit]") {
    const nlohmann::json claims = {{"id", "123"}, {"table", "users"}};
    auto result = mantis::JWT::createJWTToken(claims, "test_secret");

    REQUIRE(result.at("error").get<std::string>().empty());
    REQUIRE(!result.at("token").get<std::string>().empty());
}

TEST_CASE("JWTTest, VerifyValidToken", "[unit]") {
    const nlohmann::json claims = {{"id", "123"}, {"table", "users"}};
    auto token_result = mantis::JWT::createJWTToken(claims, "test_secret");
    auto verify_result = mantis::JWT::verifyJWTToken(
        token_result["token"], "test_secret");

    REQUIRE(verify_result.at("error").get<std::string>().empty());
    REQUIRE(verify_result["id"] == "123");
    REQUIRE(verify_result["table"] == "users");
}