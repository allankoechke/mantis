//
// Created by allan on 18/06/2025.
//
#include <gtest/gtest.h>
#include "mantis/core/jwtprovider.h"
#include <nlohmann/json.hpp>

TEST(JWTTest, CreateValidToken) {
    const nlohmann::json claims = {{"id", "123"}, {"table", "users"}};
    auto result = mantis::JWT::createJWTToken(claims, "test_secret");

    EXPECT_TRUE(result.at("error").get<std::string>().empty());
    EXPECT_FALSE(result.at("token").get<std::string>().empty());
}

TEST(JWTTest, VerifyValidToken) {
    const nlohmann::json claims = {{"id", "123"}, {"table", "users"}};
    auto token_result = mantis::JWT::createJWTToken(claims, "test_secret");
    auto verify_result = mantis::JWT::verifyJWTToken(
        token_result["token"], "test_secret");

    EXPECT_TRUE(verify_result.at("error").get<std::string>().empty());
    EXPECT_EQ(verify_result["id"], "123");
    EXPECT_EQ(verify_result["table"], "users");
}