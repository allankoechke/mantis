#include "../../../include/mantis/core/jwt.h"
#include "../../../include/mantis/app/app.h"
#include "../../../include/mantis/core/settings.h"

#include <cstring>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>

namespace mantis
{
    std::string JwtUnit::createJWTToken(const json& claims_params, const int timeout)
    {
        // Get signing key for JWT ...
        const std::string secretKey = MantisApp::jwtSecretKey();

        json res{{"error", ""}, {"token", ""}};
        if (claims_params.empty() || !claims_params.contains("id") || !claims_params.contains("table"))
        {
            res["error"] = "The claims expect 'id' and 'table' params.";
            return res;
        }

        // Give access token based on login type, `admin` or `user`
        const auto& config = MantisApp::instance().settings().configs();
        const int expiry_t = timeout > 0
                                 ? timeout // Use `timeout` value if provided
                                 : claims_params.at("table").get<std::string>() == "__admins"
                                 ? config.value("adminSessionTimeout", 1 * 60 * 60) // admins
                                 : config.value("sessionTimeout", 24 * 60 * 60); // users

        const auto time = jwt::date::clock::now();
        auto token_builder = jwt::create()
                             .set_type("JWT")
                             // .set_issuer("mantisapp") // Replace with application name
                             // .set_audience("mydomain.io") // Replace with app domain
                             .set_issued_at(time)
                             .set_not_before(time)
                             .set_expires_at(time + std::chrono::seconds(expiry_t));

        for (const auto& [key, value] : claims_params.items())
        {
            token_builder.set_payload_claim(key, value);
        }

        const std::string token = token_builder.sign(jwt::algorithm::none{});

        res["token"] = token;
        return res;
    }

    json JwtUnit::verifyJwtToken(const std::string& token)
    {
        // Response Object
        json result;
        result["error"] = "";
        result["verified"] = false;

        try
        {
            // Decode the token
            const auto decoded = jwt::decode(token);

            // Create verifier with your validation rules
            const auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::none{});
            // .with_issuer("auth0");  // TODO add issuer

            // Verify with error_code to capture errors
            std::error_code ec;
            verifier.verify(decoded, ec);

            if (ec)
            {
                // Validation failed - return error information
                result["error"] = ec.message();
                return result;
            }

            // Validation succeeded - extract all claims
            // Extract all payload claims
            for (auto& [key, value] : decoded.get_payload_json())
            {
                result[key] = value;
            }

            // Enforce existence of `id` and `table` claims
            if (!result.contains("id") || !result.contains("table"))
            {
                result["error"] = "Malformed token: Missing `id` or `table` claim field.";
                return result;
            }

            result["success"] = true;
            return result;
        }
        catch (const std::exception& e)
        {
            // Handle decoding errors (invalid token format, invalid JSON, etc.)
            result["success"] = false;
            result["error"] = e.what();
            return result;
        }
    }
};
