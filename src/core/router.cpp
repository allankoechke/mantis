#include "../../include/mantisbase/core/router.h"
#include "../../include/mantisbase/utils/utils.h"
#include "../../include/mantisbase/mantisbase.h"
#include "../../include/mantisbase/core/database_mgr.h"
#include "../../include/mantisbase/core/files_mgr.h"
#include "../../include/mantisbase/core/http.h"
#include "../../include/mantisbase/core/settings_mgr.h"

#include <cmrc/cmrc.hpp>
#include <chrono>
#include <thread>

// For thread logger
#include <spdlog/sinks/stdout_color_sinks-inl.h>
#include <spdlog/sinks/ansicolor_sink.h>

#include "mantisbase/core/exceptions.h"
#include "mantisbase/core/middlewares.h"

// Declare a mantis namespace for the embedded FS
CMRC_DECLARE(mantis);

namespace mantis {
    Router::Router() {
        // Let's fix timing initialization, set the start time to current time
        svr.set_pre_routing_handler(preRoutingHandler());

        // Add CORS headers to all responses
        svr.set_post_routing_handler(postRoutingHandler());

        svr.set_logger(routingLogger());

        // Handle preflight OPTIONS requests
        svr.Options(".*", optionsHandler());

        // Set Error Handler
        svr.set_error_handler(routingErrorHandler());

        // Add global middlewares to work across all routes
        m_globalMiddlewares.push_back(getAuthToken()); // Get auth token from the header
        m_globalMiddlewares.push_back(hydrateContextData()); // Fill request context with necessary data
    }

    Router::~Router() {
        if (svr.is_running())
            svr.stop();
    }

    bool Router::initialize() {
        {
            const auto sql = MantisBase::instance().db().session();
            const soci::rowset rows = (sql->prepare << "SELECT schema FROM _tables");

            if (sql->got_data()) {
                for (const auto &row: rows) {
                    const auto schema = row.get<nlohmann::json>("schema");

                    // Create entity based on the schema
                    Entity entity{schema};

                    // Create routes based on the entity type
                    if (entity.hasApi()) entity.createEntityRoutes();

                    // Store this object to keep alive function pointers
                    // if not, possible access violation error
                    m_entityMap.emplace(entity.name(), std::move(entity));
                }
            }
        }

        // Add admin routes
        EntitySchema admin_schema{"_admins", "auth"};
        admin_schema.removeField("name");
        admin_schema.setSystem(true);
        auto admin_entity = admin_schema.toEntity();
        admin_entity.createEntityRoutes();
        m_entityMap.emplace(admin_entity.name(), std::move(admin_entity));

        // Misc
        generateMiscEndpoints();

        return true;
    }

    bool Router::listen() {
        try {
            // Check if server can bind to port before launching
            if (!svr.is_valid()) {
                logger::critical("Server is not valid. Maybe port is in use or permissions issue.\n");
                return false;
            }

            const auto host = MantisBase::instance().host();
            const auto port = MantisBase::instance().port();

            // Launch logging/browser in separate thread after listen starts
            std::thread notifier([host, port]() -> void {
                // Wait a little for the server to be fully ready
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                auto endpoint = std::format("{}:{}", host, port);

                auto t_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                t_sink->set_level(spdlog::level::trace);
                t_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%-8l] %v");

                spdlog::logger logger("t_sink", {t_sink});
                logger.set_level(spdlog::level::trace);

                logger.info(
                    "Starting Servers: \n\t├── API Endpoints: http://{}/api/v1/ \n\t└── Admin Dashboard: http://{}/admin\n",
                    endpoint, endpoint);

                MantisBase::instance().openBrowserOnStart();
            });

            if (!svr.listen(host, port)) {
                logger::critical("Error: Failed to bind to {}:{}", host, port);
                notifier.join();
                return false;
            }

            notifier.join();
            return true;
        } catch (const std::exception &e) {
            logger::critical("Failed to start server: {}", e.what());
        } catch (...) {
            logger::critical("Failed to start server: Unknown Error");
        }

        return false;
    }

    void Router::close() {
        // MantisBase::instance().http().close();
        // m_routes.clear();
        if (svr.is_running()) {
            svr.stop();
            logger::info("HTTP Server Stopped.\n\t ...");
        }
    }

    httplib::Server &Router::server() {
        return svr;
    }

    void Router::Get(const std::string &path, const HandlerFn &handler, const Middlewares& middlewares) {
        m_routeRegistry.add("GET", path, handler, middlewares);
        globalRouteHandler("GET", path);
    }

    void Router::Post(const std::string &path, const HandlerFn &handler, const Middlewares& middlewares) {
        m_routeRegistry.add("POST", path, handler, middlewares);
        globalRouteHandler("POST", path);
    }

    void Router::Patch(const std::string &path, const HandlerFn &handler, const Middlewares& middlewares) {
        m_routeRegistry.add("PATCH", path, handler, middlewares);
        globalRouteHandler("PATCH", path);
    }

    void Router::Delete(const std::string &path, const HandlerFn &handler, const Middlewares &middlewares) {
        m_routeRegistry.add("DELETE", path, handler, middlewares);
        globalRouteHandler("DELETE", path);
    }

    json Router::addRoute(const std::string &table) {
        // TRACE_CLASS_METHOD()

        if (trim(table).empty()) {
            json res;
            res["success"] = false;
            res["error"] = "Table name can't be empty";
            return res;
        }

        try {
            const auto sql = MantisBase::instance().db().session();

            soci::row row;
            const std::string query = "SELECT id, schema FROM _tables WHERE name = :name";
            *sql << query, soci::use(table), soci::into(row);

            if (!sql->got_data()) {
                json res;
                res["success"] = false;
                res["error"] = "No table found with the name " + table;
                return res;
            }

            const auto id = row.get<std::string>("id");
            const auto name = row.get<std::string>("name");
            const auto type = row.get<std::string>("type");
            const auto hasApi = row.get<bool>("has_api");

            // If `hasApi` is set, schema is valid, then, add API endpoints
            if (const auto schema = row.get<json>("schema"); (hasApi && !schema.empty())) {
                // We need to persist this instance, else it'll be cleaned up causing a crash
                // const auto tableUnit = std::make_shared<TableUnit>(schema);
                // tableUnit->setTableName(name);
                // tableUnit->setTableId(id);
                //
                // if (!tableUnit->setupRoutes())
                //     return false;

                // m_routes.push_back(tableUnit);
            }
        } catch (const std::exception &e) {
            json res;
            res["success"] = false;
            res["error"] = e.what();
            return res;
        }

        json res;
        res["success"] = true;
        res["error"] = "";
        return res;
    }

    json Router::updateRoute(const json &table_data) {
        // TRACE_CLASS_METHOD()

        json res;
        res["success"] = false;
        res["error"] = "";

        if (table_data.is_null() || table_data.empty()) {
            res["error"] = "Table data can't be empty!";
            return res;
        }

        if (!table_data.contains("new_name") || table_data["new_name"].is_null() || table_data["new_name"].empty()) {
            res["error"] = "Table new name can't be null/empty!";
            return res;
        }

        if (!table_data.contains("old_name") || table_data["old_name"].is_null() || table_data["old_name"].empty()) {
            res["error"] = "Table old name can't be null/empty!";
            return res;
        }

        if (!table_data.contains("old_type") || table_data["old_type"].is_null() || table_data["old_type"].empty()) {
            res["error"] = "Table type is required!";
            return res;
        }

        const auto table_name = table_data.at("new_name").get<std::string>();
        const auto table_old_name = table_data.at("old_name").get<std::string>();
        const auto table_type = table_data.at("old_type").get<std::string>();

        // // Let's find and remove existing object
        // const auto it = std::ranges::find_if(m_routes, [&](const auto& route)
        // {
        //     return route->tableName() == table_old_name;
        // });
        //
        // if (it == m_routes.end())
        // {
        //     res["error"] = "TableUnit for " + table_old_name + " not found!";
        //     return res;
        // }

        // Also, check if we have defined some routes for this one ...
        const auto basePath = "/api/v1/" + table_old_name;
        m_routeRegistry.remove("GET", basePath);
        m_routeRegistry.remove("GET", basePath + "/:id");

        if (table_type != "view") {
            m_routeRegistry.remove("POST", basePath);
            m_routeRegistry.remove("PATCH", basePath + "/:id");
            m_routeRegistry.remove("DELETE", basePath + "/:id");
        }

        if (table_type == "auth") {
            m_routeRegistry.remove("POST", basePath + "/auth-with-password");
        }

        // Remove tableUnit instance for the instance
        // m_routes.erase(it);

        return addRoute(table_name);
    }

    json Router::updateRouteCache(const json &table_data) {
        // TRACE_CLASS_METHOD()

        json res;
        res["success"] = false;
        res["error"] = "";

        // Get table name
        const auto table_name = table_data.at("name").get<std::string>();

        // // Let's find and remove existing object
        // const auto it = std::ranges::find_if(m_routes, [&](const auto& route)
        // {
        //     return route->tableName() == table_name;
        // });
        //
        // if (it == m_routes.end())
        // {
        //     res["error"] = "TableUnit for " + table_name + " not found!";
        //     return res;
        // }

        // Update cached data & rules using the schema
        // (*it)->fromJson(table_data);

        res["success"] = true;
        return res;
    }

    json Router::removeRoute(const json &table_data) {
        // TRACE_CLASS_METHOD()

        json res;
        res["success"] = false;
        res["error"] = "";

        if (table_data.is_null() || table_data.empty()) {
            res["error"] = "Table data can't be empty!";
            return res;
        }

        if (!table_data.contains("name") || table_data["name"].is_null() || table_data["name"].empty()) {
            res["error"] = "Table name can't be null/empty!";
            return res;
        }

        if (!table_data.contains("type") || table_data["type"].is_null() || table_data["type"].empty()) {
            res["error"] = "Table type is required!";
            return res;
        }

        const auto table_name = table_data.at("name").get<std::string>();
        const auto table_type = table_data.at("type").get<std::string>();

        // // Let's find and remove existing object
        // const auto it = std::ranges::find_if(m_routes, [&](const auto& route)
        // {
        //     return route->tableName() == table_name;
        // });
        //
        // if (it == m_routes.end())
        // {
        //     res["error"] = "TableUnit not found!";
        //     return res;
        // }

        // Also, check if we have defined some routes for this one ...

        const auto basePath = "/api/v1/" + table_name;
        m_routeRegistry.remove("GET", basePath);
        m_routeRegistry.remove("GET", basePath + "/:id");

        if (table_type != "view") {
            m_routeRegistry.remove("POST", basePath);
            m_routeRegistry.remove("PATCH", basePath + "/:id");
            m_routeRegistry.remove("DELETE", basePath + "/:id");
        }

        if (table_type == "auth") {
            m_routeRegistry.remove("POST", basePath + "/auth-with-password");
        }

        // // Remove tableUnit instance for the instance
        // m_routes.erase(it);

        res["success"] = true;
        return res;
    }

    const json &Router::schemaCache(const std::string &table_name) const {
        if (m_entityMap.contains(table_name)) {
            return m_entityMap.at(table_name).schema();
        }

        throw std::out_of_range("Table does not contain schema");
    }

    void Router::addSchemaCache(const std::string &table_name, const nlohmann::json &table_schema) {
        if (m_entityMap.contains(table_name)) {
            updateSchemaCache(table_name, table_schema);
        } else {
            m_entityMap.insert_or_assign(table_name, Entity(table_schema));
        }
    }

    void Router::updateSchemaCache(const std::string &table_name, const json &table_schema) {
        if (!m_entityMap.contains(table_name))
            throw std::out_of_range("Cannot update, schema not found for table: " + table_name);

        // it->second = table_schema;
    }

    void Router::removeSchemaCache(const std::string &table_name) {
        m_entityMap.erase(table_name);
    }

    void Router::globalRouteHandler(const std::string &method, const std::string &path) {
        const std::function handlerFunc = [this, method, path](const httplib::Request &req, httplib::Response &res) {
            MantisRequest ma_req{req};
            MantisResponse ma_res{res};

            logger::trace("Req: [{}] {}", method, path);

            const auto route = m_routeRegistry.find(method, path);
            if (!route) {
                json response;
                response["status"] = 404;
                response["error"] = std::format("{} {} Route Not Found", method, path);
                response["data"] = json::object();

                ma_res.sendJson(404, response);
                return;
            }

            logger::trace("Route Found, executing global middlewares ...");

            // First, execute global middlewares
            for (const auto &g_mw: m_globalMiddlewares) {
                if (g_mw(ma_req, ma_res) == HandlerResponse::Handled) return;
            }

            logger::trace("Executing route specific middlewares ...");

            // Secondly, execute route specific middlewares
            for (const auto &mw: route->middlewares) {
                if (mw(ma_req, ma_res) == HandlerResponse::Handled) return;
            }

            logger::trace("Calling route handler finally ...");

            // Finally, execute the handler function
            if (const auto func = std::get_if<HandlerFn>(&route->handler)) {
                (*func)(ma_req, ma_res);
            }

            logger::trace("Route execution completed ...");
        };

        if (method == "GET") {
            svr.Get(path, handlerFunc);
        } else if (method == "PATCH") {
            svr.Patch(path, handlerFunc);
        } else if (method == "POST") {
            svr.Post(path, handlerFunc);
        } else if (method == "DELETE") {
            svr.Delete(path, handlerFunc);
        } else {
            throw MantisException(500, "Router method `" + method + "` is not supported!");
        }
    }

    void Router::generateMiscEndpoints() {
        auto &router = MantisBase::instance().router();
        router.Get("/api/v1/healthcheck", healthCheckHandler());
        router.Get("/api/files/:entity/:file", fileServingHandler());
        router.Get(R"(/admin(.*))", handleAdminDashboardRoute());

        // Add /public static file serving directory
        const auto mount_ok = svr.set_mount_point("/", MantisBase::instance().publicDir());
        if (!mount_ok) {
            logger::critical("Failed to setup mount point directory for '/' at '{}'",
                             MantisBase::instance().publicDir());
        }
    }

    std::function<void(const MantisRequest &, MantisResponse &)> Router::handleAdminDashboardRoute() const {
        return [this](const MantisRequest &req, MantisResponse &res) {
            logger::trace("ADMIN ...");
            try {
                const auto fs = cmrc::mantis::get_filesystem();
                std::string path = req.matches()[1];

                std::cout << "PATH: " << path << std::endl;

                // Normalize the path
                if (path.empty() || path == "/") {
                    path = "/qrc/index.html";
                } else {
                    path = std::format("/qrc{}", path);
                }

                if (!fs.exists(path)) {
                    logger::trace("{} path does not exists", path);

                    // fallback to index.html for React routes
                    path = "/qrc/index.html";
                }

                try {
                    const auto file = fs.open(path);
                    const auto mime = Router::getMimeType(path);
                    res.setContent(file.begin(), file.size(), mime);
                    res.setStatus(200);
                } catch (const std::exception &e) {
                    const auto file = fs.open("/qrc/404.html");
                    const auto mime = Router::getMimeType("404.html");

                    res.setContent(file.begin(), file.size(), mime);
                    res.setStatus(404);
                    logger::critical("Error processing /admin response: {}", e.what());
                }
            } catch (const std::exception &e) {
                res.setStatus(500);
                logger::critical("Error processing /admin request: {}", e.what());
            }
        };
    }

    std::function<void(const MantisRequest &, MantisResponse &)> Router::fileServingHandler() {
        logger::trace("Registering /api/files/:entity/:file GET endpoint ...");
        return [](const MantisRequest &req, MantisResponse &res) {
            std::cout << "fileServingHandler()" << std::endl;
            const auto table_name = req.getPathParamValue("entity");
            const auto file_name = req.getPathParamValue("file");

            if (table_name.empty() || file_name.empty()) {
                json response;
                response["error"] = "Both entity name and file name are required!";
                response["status"] = 400;
                response["data"] = json::object();

                res.sendJson(400, response);
                return;
            }

            const auto& fileMgr = MantisBase::instance().files();
            if (const auto path_opt = fileMgr.getFilePath(table_name, file_name);
                path_opt.has_value()) {
                // Return requested file
                res.setStatus(200);
                res.setFileContent(path_opt.value());
                return;
            }

            json response;
            response["error"] = "File not found!";
            response["status"] = 404;
            response["data"] = json::object();

            res.sendJson(404, response);
        };
    }

    std::function<void(const MantisRequest &, MantisResponse &)> Router::healthCheckHandler() {
        return [](const MantisRequest &, const MantisResponse &res) {
            // Compute system uptime and send to user
            const auto &start_time = MantisBase::instance().startTime();
            auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - start_time).count();

            json response;
            response["status"] = "OK";
            response["uptime"] = uptime;
            res.sendJson(200, response);
        };
    }

    std::string Router::getMimeType(const std::string &path) {
        if (path.ends_with(".js")) return "application/javascript";
        if (path.ends_with(".css")) return "text/css";
        if (path.ends_with(".html")) return "text/html";
        if (path.ends_with(".json")) return "application/json";
        if (path.ends_with(".png")) return "image/png";
        if (path.ends_with(".svg")) return "image/svg+xml";
        return "application/octet-stream";
    }
}
