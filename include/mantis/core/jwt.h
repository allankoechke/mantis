/**
 * @file jwt.h
 * @brief Handles JSON Web Token (JWT) creation and verification.
 *
 * Created by allan on 07/06/2025.
 */

#ifndef JWT_H
#define JWT_H

#include <string>
#include <nlohmann/json.hpp>

#include "logging.h"
#include "../utils/utils.h"

namespace mantis
{
    using json = nlohmann::json;

    /**
     * This class manages creation and verification of JWTs.
     */
    class JWT
    {
    public:
        /**
         * @brief Generate a JWT token having some custom claims.
         *
         * If JWT token creation fails, the error key of the JSON response will detail the error value.
         *
         * @param claims_params Additional claims, for now only `id` and `table` is supported.
         * @param secretKey Secret key for signing the JWT tokens.
         * @return JSON Object having a `token` and `error` value. The error or token may be empty.
         */
        static json createJWTToken(const json& claims_params, const std::string& secretKey);
        /**
         * @brief Verify if given token is valid and was created by us.
         *
         * If verification failed, the error JSON key of the response will have the error value. If everything went well,
         * all the other JSON fields will be filled with extracted values.
         *
         * @param token JWT Token
         * @param secretKey Secret key for signing the JWT tokens.
         * @return JSON Object having a `id`, `table`, `verified` and `error` values.
         */
        static json verifyJWTToken(const std::string& token, const std::string& secretKey);
    };
} // mantis

#endif //JWT_H
