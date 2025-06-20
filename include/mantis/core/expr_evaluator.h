//
// Created by allan on 09/06/2025.
//

#ifndef EXPR_EVALUATOR_H
#define EXPR_EVALUATOR_H

#include <string>
#include <shunting-yard.h>
#include <containers.h>
#include <nlohmann/json.hpp>

namespace mantis
{
    using cparse::TokenMap;
    using cparse::calculator;
    using cparse::packToken;
    using json = nlohmann::json;

    struct ExprEvaluator
    {
        ExprEvaluator();

        auto evaluate(const std::string& expr, const TokenMap& vars) -> bool;

        auto evaluate(const std::string& expr, const json& vars) -> bool;

        auto jsonToTokenMap(const json& j) -> TokenMap;
    };
} // mantis

#endif //EXPR_EVALUATOR_H
