//
// Created by codeart on 12/11/2025.
//

#ifndef MANTISBASE_MIDDLEWARES_H
#define MANTISBASE_MIDDLEWARES_H


namespace mantis {
    class MantisResponse;
    class MantisRequest;

    bool getAuthToken(MantisRequest& req, MantisResponse& res);
    bool hasAccess(MantisRequest& req, MantisResponse& res);
}

#endif //MANTISBASE_MIDDLEWARES_H