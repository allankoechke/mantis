//
// Created by codeart on 12/11/2025.
//

#ifndef MANTISBASE_MIDDLEWARES_H
#define MANTISBASE_MIDDLEWARES_H

#include <functional>
#include <string>

namespace mantis {
    class MantisResponse;
    class MantisRequest;

    std::function<bool(MantisRequest&, MantisResponse&)> getAuthToken();
    std::function<bool(MantisRequest&, MantisResponse&)> hasAccess();
    std::function<bool(MantisRequest&, MantisResponse&)> requireExprEval(const std::string& expr);
    std::function<bool(MantisRequest&, MantisResponse&)> requireGuestOnly();
    std::function<bool(MantisRequest&, MantisResponse&)> requireAdminAuth();
    std::function<bool(MantisRequest&, MantisResponse&)> requireAdminOrEntityAuth(const std::string& entity_name);
    std::function<bool(MantisRequest&, MantisResponse&)> requireEntityAuth(const std::string& entity_name);
}

#endif //MANTISBASE_MIDDLEWARES_H