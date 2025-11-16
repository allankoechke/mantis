#include "../../include/mantis/core/router.h"
#include "../../include/mantis/utils/utils.h"
#include "../../include/mantis/mantisbase.h"
#include "../../include/mantis/core/database_mgr.h"
// #include "../../include/mantis/core/tables/tables.h"
// #include "../../include/mantis/core/tables/sys_tables.h"
#include "../../include/mantis/core/files_mgr.h"
#include "../../include/mantis/core/http.h"
#include "../../include/mantis/core/settings_mgr.h"

#include <cmrc/cmrc.hpp>

#include <chrono>
#include <thread>

// For thread logger
#include <spdlog/sinks/stdout_color_sinks-inl.h>
#include <spdlog/sinks/ansicolor_sink.h>

#include "mantis/core/exceptions.h"

// Declare a mantis namespace for the embedded FS
CMRC_DECLARE(mantis);

#define __file__ "core/router.cpp"

namespace mantis {
    Router::Router() {
        // Let's fix timing initialization, set the start time to current time
        svr.set_pre_routing_handler([](const httplib::Request &req, httplib::Response &res) {
            auto &mutable_req = const_cast<httplib::Request &>(req);
            mutable_req.start_time_ = std::chrono::steady_clock::now(); // Set the start time
            return httplib::Server::HandlerResponse::Unhandled;
        });

        // Add CORS headers to all responses
        svr.set_post_routing_handler([](const auto &req, auto &res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PATCH, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            res.set_header("Access-Control-Max-Age", "86400");
        });

        // svr.set_logger([](const auto& req, const auto& res)
        // {
        //     // Calculate execution time (if start_time was set)
        //     const auto end_time = std::chrono::steady_clock::now();
        //     const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        //         end_time - req.start_time_).count();
        //
        //     if (res.status < 400)
        //     {
        //         logger::info("{} {:<7} {}  - Status: {}  - Time: {}ms",
        //                   req.version, req.method, req.path, res.status, duration_ms);
        //     }
        //     else
        //     {
        //         // Decompress if content is compressed
        //         if (res.body.empty())
        //         {
        //             logger::info("{} {:<7} {}  - Status: {}  - Time: {}ms",
        //                       req.version, req.method, req.path, res.status, duration_ms);
        //         }
        //         else
        //         {
        //             // Get the compression encoding
        //             const std::string encoding = res.get_header_value("Content-Encoding");
        //
        //             auto body = encoding.empty() ? res.body : decompressResponseBody(res.body, encoding);
        //             logger::info("{} {:<7} {}  - Status: {}  - Time: {}ms\n\t└──Body: {}",
        //                       req.version, req.method, req.path, res.status, duration_ms, body);
        //         }
        //     }
        // });

        // Handle preflight OPTIONS requests
        svr.Options(".*", [](const auto &req, auto &res) {
            // Headers are already set by post_routing_handler
            res.status = 200;
        });

        // Set Error Handler
        svr.set_error_handler([]([[maybe_unused]] const httplib::Request &req, httplib::Response &res) {
            if (res.body.empty()) {
                json response;
                response["status"] = res.status;
                response["data"] = json::object();

                if (res.status == 404)
                    response["error"] = "Resource not found!";
                else if (res.status >= 500)
                    response["error"] = "Internal server error, try again later!";
                else
                    response["error"] = "Something went wrong here!";

                res.set_content(response.dump(), "application/json");
            }
        });
    }

    Router::~Router() {
        if (svr.is_running()) svr.stop();
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

    void Router::Get(const std::string &path, HandlerFn handler, Middlewares middlewares) {
        m_routeRegistry.add("GET", path, handler, middlewares);
        globalRouteHandler("GET", path);
    }

    void Router::Post(const std::string &path, HandlerFn handler, Middlewares middlewares) {
        m_routeRegistry.add("POST", path, handler, middlewares);
        globalRouteHandler("POST", path);
    }

    void Router::Patch(const std::string &path, HandlerFn handler, Middlewares middlewares) {
        m_routeRegistry.add("PATCH", path, handler, middlewares);
        globalRouteHandler("PATCH", path);
    }

    void Router::Delete(const std::string &path, HandlerFn handler, Middlewares middlewares) {
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

            const auto route = m_routeRegistry.find(req.method, req.path);
            if (!route) {
                json response;
                response["status"] = 404;
                response["error"] = std::format("{} {} Route Not Found", method, path);
                response["data"] = json::object();

                ma_res.sendJson(404, response);
                return;
            }

            // First, execute global middlewares
            for (const auto &g_mw: m_globalMiddlewares) {
                if (!g_mw(ma_req, ma_res)) return;
            }

            // Secondly, execute route specific middlewares
            for (const auto &mw: route->middlewares) {
                if (!mw(ma_req, ma_res)) return;
            }

            // TODO ...
            // Finally, execute the handler function
            if (const auto func = std::get_if<HandlerFn>(&route->handler)) {
                (*func)(ma_req, ma_res);
            }
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

    void Router::generateMiscEndpoints() const {
        logger::trace("Generate MiscEndpoints");
        // Add /health for server health check
        auto &router = MantisBase::instance().router();
        router.Get("/api/v1/healthcheck",
                   [](MantisRequest &, const MantisResponse &res) {
                logger::trace("HEALTH ...");
                       // Compute system uptime and send to user
                       const auto &start_time = MantisBase::instance().startTime();
                       auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::steady_clock::now() - start_time).count();

                       json response;
                       response["status"] = "ok";
                       response["uptime"] = uptime;
                       res.sendJson(200, response);
                   }
        );

        router.Get(
            R"(/admin(.*))", [](const MantisRequest &req, MantisResponse &res) {
                logger::trace("ADMIN ...");
                try {
                    const auto fs = cmrc::mantis::get_filesystem();
                    std::string path = req.matches()[1];

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
            });

        // Add /public static file serving directory
        const auto mount_ok = router.server().set_mount_point("/", MantisBase::instance().publicDir());
        if (!mount_ok) {
            logger::critical("Failed to setup mount point directory for '/' at '{}'",
                             MantisBase::instance().publicDir());
        }
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

    // bool Router::generateFileServingApi() const {
    //     try {
    //         Get(
    //             "/api/files/:table/:filename",
    //             [](const MantisRequest &req, MantisResponse &res) {
    //                 const auto table_name = req.getPathParamValue("table");
    //                 const auto file_name = req.getPathParamValue("filename");
    //
    //                 if (table_name.empty() || file_name.empty()) {
    //                     json response;
    //                     response["error"] = "Table name and file name are required!";
    //                     response["status"] = "400";
    //                     response["data"] = json::object();
    //
    //                     res.sendJson(400, response);
    //                     return;
    //                 }
    //
    //                 const auto fileMgr = MantisBase::instance().files();
    //                 if (const auto path_opt = fileMgr.getFilePath(table_name, file_name);
    //                     path_opt.has_value()) {
    //                     // Return requested file
    //                     res.setStatus(200);
    //                     res.setFileContent(path_opt.value());
    //                     return;
    //                 }
    //
    //                 json response;
    //                 response["error"] = "File not found!";
    //                 response["status"] = "404";
    //                 response["data"] = json::object();
    //
    //                 res.sendJson(404, response);
    //             }
    //         );
    //
    //         return true;
    //     } catch (std::exception &e) {
    //         logger::critical("Error creating file serving endpoint: {}", e.what());
    //     }
    //
    //     return false;
    // }
    //
    // bool Router::generateAdminCrudApis() const {
    //     TRACE_CLASS_METHOD()
    //
    //     try {
    //         // Setup routes for the admin users fetch/create/update/delete
    //         if (!m_adminTable->setupRoutes()) {
    //             logger::critical("Failed to setup admin table routes");
    //             return false;
    //         }
    //
    //         // Setup routes to manage database tables fetch/create/update/delete
    //         if (!m_tableRoutes->setupRoutes()) {
    //             logger::critical("Failed to setup database table routes");
    //             return false;
    //         }
    //
    //         if (!MantisBase::instance().settings().setupRoutes()) {
    //             logger::critical("Failed to setup settings routes");
    //             return false;
    //         }
    //

    //     } catch (const std::exception &e) {
    //         logger::critical("Error creating admin routes: ", e.what());
    //         return false;
    //     }
    //
    //     return true;
    // }
}
