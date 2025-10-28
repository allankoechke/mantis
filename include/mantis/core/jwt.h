/**
 * @file jwt.h
 * @brief Handles JSON Web Token (JWT) creation and verification.
 *
 * Created by allan on 07/06/2025.
 */

#ifndef JWTUNIT_H
#define JWTUNIT_H

#include <string>
#include "../utils/utils.h"

namespace mantis
{
    /**
     * This class manages creation and verification of JWTs.
     */
    class JwtUnit
    {
    public:
        /**
         * @brief Generate a JWT token having some custom claims.
         *
         * If JWT token creation fails, the error key of the JSON response will detail the error value.
         *
         * @param claims_params Additional claims, for now only `id` and `table` is supported.
         * @param timeout Duration in seconds to keep the tokens valid.
         * @return String `token` value if all went well.
         */
        static std::string createJWTToken(const json& claims_params = json::object(), int timeout = 86400);
        /**
         * @brief Verify if given token is valid and was created by us.
         *
         * If verification failed, the error JSON key of the response will have the error value. If everything went well,
         * all the other JSON fields will be filled with extracted values.
         *
         * @param token JWT Token
         * @return JSON Object having a `id`, `table`, `verified` and `error` values.
         */
        static json verifyJwtToken(const std::string& token);
    };
} // mantis

#endif //JWTUNIT_H
