//
// Created by allan on 07/06/2025.
//

#ifndef JWT_H
#define JWT_H

#include <string>
#include <nlohmann/json.hpp>

#include "logging.h"

namespace mantis
{
    using json = nlohmann::json;

    class JWT
    {
    public:
        static json createJWTToken(const json& claims_params, const std::string& secretKey);
        static json verifyJWTToken(const std::string& token, const std::string& secretKey);
    };
} // mantis

#endif //JWT_H
