//
// Created by codeart on 16/11/2025.
//

#include "../../include/mantis/core/models/entity.h"
#include "spdlog/fmt/bundled/std.h"

namespace mantis {
    HandlerFn Entity::getOneRouteHandler() const {
        HandlerFn handler = [this](const MantisRequest &req, const MantisResponse &res) {
            try {
                const auto entity_id = trim(req.getPathParamValue("id"));
                if (entity_id.empty())
                    throw MantisException(400, "Entity `id` is required!");

                const auto record = read(entity_id);
                res.sendJson(200,
                             {
                                 {"data", record},
                                 {"error", ""},
                                 {"status", 200}
                             }
                );
            } catch (const std::exception &e) {
                res.sendJson(500, {
                                 {"data", json::object()},
                                 {"error", e.what()},
                                 {"status", 500}
                             }
                );
            }
        };

        return handler;
    }

    HandlerFn Entity::getManyRouteHandler() const {
        HandlerFn handler = [this](MantisRequest &req, MantisResponse &res) {
            try {
                // nlohmann::json opts;
                // opts["pagination"] = {
                //     {"page_index", 1},
                //     {"per_page", 100},
                //     {"skip_total_count", false}
                // };

                const auto records = list();
                res.sendJson(200, {
                                 {"data", records},
                                 {"error", ""},
                                 {"status", 200}
                             }
                );
            } catch (const std::exception &e) {
                res.sendJson(500, {
                                 {"data", json::object()},
                                 {"error", e.what()},
                                 {"status", 500}
                             }
                );
            }
        };

        return handler;
    }

    HandlerFn Entity::postRouteHandler() const {
        HandlerFn handler = [this](MantisRequest &req, MantisResponse &res) {
        };

        return handler;
    }

    HandlerFn Entity::patchRouteHandler() const {
        HandlerFn handler = [this](MantisRequest &req, MantisResponse &res) {
            const auto entity_id = trim(req.getPathParamValue("id"));

            if (entity_id.empty())
                throw MantisException(400, "Entity `id` is required!");

            const auto &[body, err] = req.getBodyAsJson();
            if (!err.empty()) {
                res.sendJson(500, {
                                 {"data", json::object()},
                                 {"error", err},
                                 {"status", 400}
                             });
            }

            if (const auto &val_err = Validators::validateRequestBody(*this, body);
                val_err.has_value()) {
                res.sendJson(400, {
                                 {"data", json::object()},
                                 {"error", val_err.value()},
                                 {"status", 400}
                             });
                return;
            }

            const auto record = update(entity_id, body);
            res.sendJson(200, record);
            return;
        };

        return handler;
    }

    HandlerFn Entity::deleteRouteHandler() const {
        HandlerFn handler = [this](MantisRequest &req, MantisResponse &res) {
            const auto entity_id = trim(req.getPathParamValue("id"));
            if (entity_id.empty())
                throw MantisException(400, "Entity `id` is required!");

            remove(entity_id);
            res.sendEmpty();
            return;
        };

        return handler;
    }

    HandlerFn Entity::authRouteHandler() const {
        HandlerFn handler = [this](MantisRequest &req, MantisResponse &res) {
            const auto &[body, err] = req.getBodyAsJson();
            if (!err.empty()) {
                res.sendJson(400, {
                                 {"data", json::object()},
                                 {"error", err},
                                 {"status", 400}
                             });
            }

            // Validate auth fields, `email` & `password`
            // if (!body.contains("email") || !body.contains("password")) {
            //
            // }
            //
            // const auto record = update(entity_id, body);
            // res.sendJson(200, record);
            return;
        };

        return handler;
    }

    void Entity::createEntityRoutes() const {
        auto &router = MantisBase::instance().router();
        // All types do /get /get/:id
        router.Get("/api/v1/" + name(), getManyRouteHandler());
        router.Get("/api/v1/" + name() + "/:id", getOneRouteHandler());

        // base & auth /post/:id /delete/:id
        if (type() == "base" || type() == "auth") {
            router.Post("/api/v1/" + name(), postRouteHandler()); // Create Entity
            router.Patch("/api/v1/" + name() + "/:id", patchRouteHandler()); // Update Entity
            router.Delete("/api/v1/" + name() + "/:id", deleteRouteHandler()); // Delete Entity
        }

        // auth /login
        if (type() == "auth") {
            router.Post("/api/v1/" + name(), authRouteHandler());
        }
    };
}
