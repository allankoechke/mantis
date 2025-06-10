//
// Created by allan on 07/06/2025.
//

#ifndef JWT_H
#define JWT_H

#include <string>
#include <l8w8jwt/encode.h>
#include <nlohmann/json.hpp>
#include "l8w8jwt/decode.h"
#include "l8w8jwt/claim.h"
#include <cstring>

namespace mantis
{
    using json = nlohmann::json;

    class JWT
    {
    public:
        static json createJWTToken(const json& claims_params, const std::string& secretKey)
        {
            json res{{"error", ""}, {"token", ""}};
            if (claims_params.empty() || !claims_params.contains("id") || !claims_params.contains("table"))
            {
                res["error"] = "The claims expect 'id' and 'table' params.";
                return res;
            }

            char* jwt = nullptr;
            size_t jwt_length = 0;

            // Initialize encoding parameters
            l8w8jwt_encoding_params params;
            l8w8jwt_encoding_params_init(&params);

            // Set algorithm (using HS256 for simplicity)
            params.alg = L8W8JWT_ALG_HS256;

            // Set expiration to 24 hours from now (86400 seconds)
            params.iat = l8w8jwt_time(nullptr);
            params.exp = l8w8jwt_time(nullptr) + 86400;

            // Set secret key
            params.secret_key = (unsigned char*)secretKey.c_str();
            params.secret_key_length = secretKey.length();

            // Create additional payload claims for 'id' and 'table'
            l8w8jwt_claim additional_claims[2];

            const auto id = const_cast<char*>(claims_params.value("id", "").c_str());
            const auto table = const_cast<char*>(claims_params.value("table", "").c_str());

            std::cout << id << " : " << table << std::endl;

            additional_claims[0] = {
                .key = const_cast<char*>("id"),
                .key_length = 2,
                .value = id,
                .value_length = strlen(id),
                .type = L8W8JWT_CLAIM_TYPE_STRING
            };

            additional_claims[1] = {
                .key = const_cast<char*>("table"),
                .key_length = strlen("table"),
                .value = table,
                .value_length = strlen(table),
                .type = L8W8JWT_CLAIM_TYPE_STRING
            };

            params.additional_payload_claims = additional_claims;
            params.additional_payload_claims_count = 2;

            // Set output parameters
            params.out = &jwt;
            params.out_length = &jwt_length;

            // Encode the JWT
            if (const int result = l8w8jwt_encode(&params); result == L8W8JWT_SUCCESS && jwt != nullptr)
            {
                std::string token(jwt);
                l8w8jwt_free(jwt); // Always free the allocated memory
                res["token"] = token;
                return res;
            }

            // Handle encoding failure
            if (jwt != nullptr)
            {
                l8w8jwt_free(jwt);
                res["error"] = "JWT Token Encoding Error";
                return res;
            }

            res["error"] = "Could not provision an access token!";
            return res; // Return empty string on failure
        }

        static json verifyJWTToken(const std::string& token, const std::string& secretKey)
        {
            // Response Object
            json res{
                {"error", ""},
                {"verified", false},
                {"id", ""},
                {"table", ""}
            };

            // Initialize decoding parameters
            l8w8jwt_decoding_params params;
            l8w8jwt_decoding_params_init(&params);

            // Set basic parameters
            params.alg = L8W8JWT_ALG_HS256;
            params.jwt = const_cast<char*>(token.c_str());
            params.jwt_length = token.length();
            params.verification_key = (unsigned char*)secretKey.c_str();
            params.verification_key_length = secretKey.length();

            // Enable expiration validation
            params.validate_exp = 1;
            params.exp_tolerance_seconds = 0; // No tolerance for expired tokens

            l8w8jwt_validation_result validation_result;
            l8w8jwt_claim* claims = nullptr;
            size_t claims_length = 0;

            // Decode and validate the JWT
            int decode_result = l8w8jwt_decode(&params, &validation_result, &claims, &claims_length);
            std::string errorMessage;

            // Check for decoding errors first
            if (decode_result != L8W8JWT_SUCCESS)
            {
                switch (decode_result)
                {
                case L8W8JWT_DECODE_FAILED_INVALID_TOKEN_FORMAT:
                    errorMessage = "Invalid JWT structure - malformed token";
                    break;
                case L8W8JWT_DECODE_FAILED_MISSING_SIGNATURE:
                    errorMessage = "Invalid JWT structure - missing signature";
                    break;
                case L8W8JWT_BASE64_FAILURE:
                    errorMessage = "Invalid JWT structure - base64 decoding failed";
                    break;
                case L8W8JWT_NULL_ARG:
                    errorMessage = "Invalid arguments provided for verification";
                    break;
                case L8W8JWT_INVALID_ARG:
                    errorMessage = "Invalid verification key or parameters";
                    break;
                default:
                    errorMessage = "JWT decoding failed with error code: " + std::to_string(decode_result);
                }

                res["error"] = errorMessage;
                return res;
            }

            // Check validation results for specific failures
            if (validation_result != L8W8JWT_VALID)
            {
                if (validation_result & L8W8JWT_SIGNATURE_VERIFICATION_FAILURE)
                {
                    errorMessage = "JWT signature verification failed - token may be tampered with";
                }
                else if (validation_result & L8W8JWT_EXP_FAILURE)
                {
                    errorMessage = "JWT token has expired";
                }
                else if (validation_result & L8W8JWT_NBF_FAILURE)
                {
                    errorMessage = "JWT token is not yet valid (nbf claim)";
                }
                else if (validation_result & L8W8JWT_IAT_FAILURE)
                {
                    errorMessage = "JWT token issued in the future (iat claim)";
                }
                else if (validation_result & L8W8JWT_ISS_FAILURE)
                {
                    errorMessage = "JWT issuer validation failed";
                }
                else if (validation_result & L8W8JWT_SUB_FAILURE)
                {
                    errorMessage = "JWT subject validation failed";
                }
                else if (validation_result & L8W8JWT_AUD_FAILURE)
                {
                    errorMessage = "JWT audience validation failed";
                }
                else if (validation_result & L8W8JWT_JTI_FAILURE)
                {
                    errorMessage = "JWT ID validation failed";
                }
                else if (validation_result & L8W8JWT_TYP_FAILURE)
                {
                    errorMessage = "JWT type validation failed";
                }
                else
                {
                    errorMessage = "JWT validation failed with flags: " + std::to_string(validation_result);
                }

                if (claims)
                {
                    l8w8jwt_free_claims(claims, claims_length);
                }

                res["error"] = errorMessage;
                return res;
            }

            // Validate required claims
            bool foundId = false, foundTable = false;

            for (size_t i = 0; i < claims_length; i++)
            {
                if (strncmp(claims[i].key, "id", 2) == 0 && claims[i].key_length == 2)
                {
                    if (claims[i].type != L8W8JWT_CLAIM_TYPE_STRING ||
                        claims[i].value_length == 0 ||
                        claims[i].value == nullptr)
                    {
                        errorMessage = "Required claim 'id' is missing, empty, or not a string";
                        l8w8jwt_free_claims(claims, claims_length);

                        res["error"] = errorMessage;
                        return res;
                    }
                    auto id = std::string(claims[i].value, claims[i].value_length);
                    foundId = true;
                    res["id"] = id;
                }
                else if (strncmp(claims[i].key, "table", 5) == 0 && claims[i].key_length == 5)
                {
                    if (claims[i].type != L8W8JWT_CLAIM_TYPE_STRING ||
                        claims[i].value_length == 0 ||
                        claims[i].value == nullptr)
                    {
                        errorMessage = "Required claim 'table' is missing, empty, or not a string";
                        l8w8jwt_free_claims(claims, claims_length);

                        res["error"] = errorMessage;
                        return res;
                    }

                    auto table = std::string(claims[i].value, claims[i].value_length);
                    foundTable = true;
                    res["table"] = table;
                }
            }

            if (!foundId)
            {
                errorMessage = "Required claim 'id' is missing from token";
                l8w8jwt_free_claims(claims, claims_length);

                res["error"] = errorMessage;
                return res;
            }

            if (!foundTable)
            {
                errorMessage = "Required claim 'table' is missing from token";
                l8w8jwt_free_claims(claims, claims_length);

                res["error"] = errorMessage;
                return res;
            }

            // All validations passed
            l8w8jwt_free_claims(claims, claims_length);


            res["error"] = "";
            res["verified"] = true;
            return res;
        }
    };
} // mantis

#endif //JWT_H
