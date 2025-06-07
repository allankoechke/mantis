//
// Created by allan on 07/06/2025.
//

#ifndef JWT_H
#define JWT_H

#include <string>
#include <l8w8jwt/encode.h>
#include <nlohmann/json.hpp>

namespace mantis {
    using json = nlohmann::json;

class JWT {
public:
    static json createJWT(const std::string& secretKey, const json& claims_params)
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
        uint8_t ind = 0;
        for (const auto& [k, v] : claims_params.items())
        {
            const auto key = const_cast<char*>(k.c_str());
            const auto val = const_cast<char*>(v.get<std::string>().c_str());

            additional_claims[ind] = {
                .key = key,
                .key_length = 2,
                .value = val,
                .value_length = v.get<std::string>().length(),
                .type = L8W8JWT_CLAIM_TYPE_STRING
            };
            ind++;
        }

        params.additional_payload_claims = additional_claims;
        params.additional_payload_claims_count = 2;

        // Set output parameters
        params.out = &jwt;
        params.out_length = &jwt_length;

        // Encode the JWT
        if (const int result = l8w8jwt_encode(&params); result == L8W8JWT_SUCCESS && jwt != nullptr) {
            std::string token(jwt);
            l8w8jwt_free(jwt);  // Always free the allocated memory
            res["token"] = token;
            return res;
        }

        // Handle encoding failure
        if (jwt != nullptr) {
            l8w8jwt_free(jwt);
            res["error"] = "JWT Token Encoding Error";
            return res;
        }

        res["error"] = "Could not provision an access token!";
        return res; // Return empty string on failure
    }

};

} // mantis

#endif //JWT_H
