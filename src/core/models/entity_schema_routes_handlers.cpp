#include "../../include/mantisbase/core/models/entity.h"
// #include "spdlog/fmt/bundled/std.h"
#include "../../include/mantisbase/core/auth.h"

namespace mantis {
    HandlerFn EntitySchema::getOneRouteHandler() const {
        HandlerFn handler = [](const MantisRequest &req, const MantisResponse &res) {
            try {
                // Extract path `id` value [REQUIRED]
                const auto schema_id = trim(req.getPathParamValue("schema_name_or_id"));
                if (schema_id.empty())
                    throw MantisException(400, "Entity `id` or `schema_name` is required!");

                // Read for entity matching given `id` if it exists, return it, else error `404`
                const auto record = EntitySchema::getTable(schema_id);
                    res.sendJson(200,
                                 {
                                     {"data", record},
                                     {"error", ""},
                                     {"status", 200}
                                 }
                    );
            } catch (const MantisException &e) {
                res.sendJson(e.code(), {
                                 {"data", json::object()},
                                 {"error", e.what()},
                                 {"status", e.code()}
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

    HandlerFn EntitySchema::getManyRouteHandler() const {
        HandlerFn handler = [](MantisRequest &req, MantisResponse &res) {
            try {

                const auto tables = EntitySchema::listTables();
                res.sendJson(200, {
                                 {"data", tables },
                                 {"error", ""},
                                 {"status", 200}
                             }
                );
            } catch (const MantisException &e) {
                res.sendJson(e.code(), {
                                 {"data", json::object()},
                                 {"error", e.what()},
                                 {"status", e.code()}
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

    HandlerFn EntitySchema::postRouteHandler() const {
        // Capture entity name, currently we get an error if we capture `this` directly.
        const std::string entity_name = name();

        HandlerFn handler = [entity_name](MantisRequest &req, MantisResponse &res) {
            // Get entity object for given name
            const auto entity = MantisBase::instance().entity(entity_name);
        };

        return handler;
    }

    HandlerFn EntitySchema::patchRouteHandler() const {
        HandlerFn handler = [](MantisRequest &req, MantisResponse &res) {
            // const auto entity_id = trim(req.getPathParamValue("id"));
            //
            // if (entity_id.empty())
            //     throw MantisException(400, "Entity `id` is required!");
            //
            // const auto &[body, err] = req.getBodyAsJson();
            // if (!err.empty()) {
            //     res.sendJson(500, {
            //                      {"data", json::object()},
            //                      {"error", err},
            //                      {"status", 400}
            //                  });
            // }
            //
            // if (const auto &val_err = Validators::validateRequestBody(entity, body);
            //     val_err.has_value()) {
            //     res.sendJson(400, {
            //                      {"data", json::object()},
            //                      {"error", val_err.value()},
            //                      {"status", 400}
            //                  });
            //     return;
            // }
            //
            // const auto record = entity.update(entity_id, body);
            // res.sendJson(200, record);
            // return;
        };

        return handler;
    }

    HandlerFn EntitySchema::deleteRouteHandler() const {
        HandlerFn handler = [](MantisRequest &req, MantisResponse &res) {
            // const auto entity_id = trim(req.getPathParamValue("id"));
            // if (entity_id.empty())
            //     throw MantisException(400, "Entity `id` is required!");
            //
            // entity.remove(entity_id);
            // res.sendEmpty();
        };

        return handler;
    }

    void EntitySchema::createEntityRoutes() const {
        auto &router = MantisBase::instance().router();

        router.Get("/api/v1/schemas", getManyRouteHandler());
        router.Post("/api/v1/schemas", postRouteHandler()); // Create Entity
        router.Get("/api/v1/schemas/:schema_name_or_id", getOneRouteHandler());
        router.Patch("/api/v1/schemas/:schema_name_or_id", patchRouteHandler()); // Update Entity
        router.Delete("/api/v1/schemas/:schema_name_or_id", deleteRouteHandler()); // Delete Entity
    };
}
