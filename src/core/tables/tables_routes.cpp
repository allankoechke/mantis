//
// Created by allan on 07/06/2025.
//

#include <fstream>

#include "../../include/mantis/core/tables/tables.h"
#include "../../include/mantis/app/app.h"
#include "../../include/mantis/core/database.h"
#include "../../../include/mantis/utils/utils.h"
#include "mantis/core/fileunit.h"
#include <iostream>

#define __file__ "core/tables/tables_routes.cpp"

namespace mantis
{
    bool TableUnit::setupRoutes()
    {
        if (m_tableName.empty() && m_routeName.empty()) return false;

        const auto path = m_routeName.empty() ? m_tableName : m_routeName;
        const auto basePath = "/api/v1/" + path;

        try
        {
            // Fetch All Records
            Log::debug("Adding GET Request for table '{}'", basePath);
            MantisApp::instance().http().Get(
                basePath,
                [this](const Request& req, Response& res, Context& ctx)-> void
                {
                    fetchRecords(req, res, ctx);
                },
                {
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return getAuthToken(req, res, ctx);
                    },
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return hasAccess(req, res, ctx);
                    }
                }
            );

            // Fetch Single Record
            Log::debug("Adding GET/1 Request for table '{}'", basePath);
            MantisApp::instance().http().Get(
                basePath + "/:id",
                [this](const Request& req, Response& res, Context& ctx)-> void
                {
                    fetchRecord(req, res, ctx);
                },
                {
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return getAuthToken(req, res, ctx);
                    },
                    [this](const Request& req, Response& res, Context& ctx)-> bool
                    {
                        return hasAccess(req, res, ctx);
                    }
                }
            );

            // Add/Update and Delete are not supported in views
            if (m_tableType != "view")
            {
                // Add Record
                Log::debug("Adding POST Request for table '{}'", basePath);
                MantisApp::instance().http().Post(
                    basePath, [this](const Request& req, Response& res, const ContentReader& reader,
                                     Context& ctx)-> void
                    {
                        createRecord(req, res, reader, ctx);
                    },
                    {
                        [this](const Request& req, Response& res, Context& ctx)-> bool
                        {
                            return getAuthToken(req, res, ctx);
                        },
                        [this](const Request& req, Response& res, Context& ctx)-> bool
                        {
                            return hasAccess(req, res, ctx);
                        }
                    }
                );

                // Update Record
                Log::debug("Adding PATCH Request for table '{}'", basePath);
                MantisApp::instance().http().Patch(
                    basePath + "/:id",
                    [this](const Request& req, Response& res, const ContentReader& reader, Context& ctx)-> void
                    {
                        updateRecord(req, res, reader, ctx);
                    },
                    {
                        [this](const Request& req, Response& res, Context& ctx)-> bool
                        {
                            return getAuthToken(req, res, ctx);
                        },
                        [this](const Request& req, Response& res, Context& ctx)-> bool
                        {
                            return hasAccess(req, res, ctx);
                        }
                    }
                );

                // Delete Record
                Log::debug("Adding DELETE Request for table '{}'", basePath);
                MantisApp::instance().http().Delete(
                    basePath + "/:id",
                    [this](const Request& req, Response& res, Context& ctx)-> void
                    {
                        deleteRecord(req, res, ctx);
                    },
                    {
                        [this](const Request& req, Response& res, Context& ctx)-> bool
                        {
                            return getAuthToken(req, res, ctx);
                        },
                        [this](const Request& req, Response& res, Context& ctx)-> bool
                        {
                            return hasAccess(req, res, ctx);
                        }
                    }
                );
            }

            // Add auth endpoints for login
            if (m_tableType == "auth")
            {
                // Add Record
                Log::debug("Adding POST Request for table '{}/auth-with-password'", basePath);
                MantisApp::instance().http().Post(
                    basePath + "/auth-with-password",
                    [this](const Request& req, Response& res, Context& ctx) -> void
                    {
                        authWithEmailAndPassword(req, res, ctx);
                    }
                );
            }

            return true;
        }

        catch (const std::exception& e)
        {
            Log::critical("Failed to create routes for table '{}' of '{}' type: {}",
                          m_tableName, m_tableType, e.what());
            return false;
        }

        catch (...)
        {
            Log::critical("Failed to create routes for table '{}' of '{}' type: {}",
                          m_tableName, m_tableType, "Unknown Error!");
            return false;
        }
    }

    void TableUnit::fetchRecord(const Request& req, Response& res, Context& ctx)
    {
        Log::trace("Fetching record from endpoint {}", req.path);

        // Extract request ID and check that it's not empty
        const auto id = req.path_params.at("id");

        json response;
        if (id.empty())
        {
            response["status"] = 400;
            response["error"] = "Record ID is required";
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }

        try
        {
            // For every read, check that the optional<T> has a value.
            // If it's not null, get the data and respond back to the client
            // else, handle the 404 NOT FOUND response to the client
            if (const auto resp = read(id, json::object()); resp.has_value())
            {
                response["status"] = 200;
                response["error"] = "";
                response["data"] = resp.value();

                res.set_content(response.dump(), "application/json");
                res.status = 200;
                return;
            }

            response["status"] = 404;
            response["error"] = "Item Not Found";
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 404;
        }

        // For any server errors, send it back to the client
        // Capture std::exception, and a catch-all block as well
        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["error"] = e.what();
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 500;
        }

        catch (...)
        {
            response["status"] = 500;
            response["error"] = "Unknown Error";
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = 500;
        }
    }

    void TableUnit::fetchRecords(const Request& req, Response& res, Context& ctx)
    {
        Log::trace("Fetching all record from endpoint {}", req.path);

        // Pagination req params
        json pagination;
        pagination["perPage"] = 100; // Number of records per page
        pagination["pageIndex"] = 1; // Current page, 1-index based
        pagination["pageCount"] = -1; // Total pages
        pagination["recordCount"] = 0; // Total pages
        pagination["countPages"] = true; // Whether to calculate number of pages

        if (req.has_param("perPage"))
            pagination["perPage"] = std::stoi(req.get_param_value("perPage"));

        if (req.has_param("pageIndex"))
            pagination["pageIndex"] = std::stoi(req.get_param_value("pageIndex"));

        if (req.has_param("countPages"))
        {
            const std::string value = req.get_param_value("countPages");
            pagination["countPages"] = value.empty() ? true : strToBool(value);
        }

        json response;
        try
        {
            json opts;
            opts["pagination"] = pagination;
            const auto resp = list_records(opts);
            if (const auto err = resp.value("error", ""); !err.empty())
            {
                response["data"] = json::array();
                response["status"] = 400;
                response["error"] = err;

                res.status = 400;
                res.set_content(response.dump(), "application/json");
            }

            response["data"] = resp["data"];
            response["pagination"] = resp["pagination"];
            response["status"] = 200;
            response["error"] = "";

            res.status = 200;
            res.set_content(response.dump(), "application/json");
        }

        catch (const std::exception& e)
        {
            response["data"] = json::array();
            response["status"] = 500;
            response["error"] = e.what();

            res.status = 500;
            res.set_content(response.dump(), "application/json");
        }

        catch (...)
        {
            response["data"] = json::array();
            response["status"] = 500;
            response["error"] = "Internal Server Error";

            res.status = 500;
            res.set_content(response.dump(), "application/json");
        }
    }

    void TableUnit::createRecord(const Request& req, Response& res, const ContentReader& reader, Context& ctx)
    {
        Log::trace("Creating new record, endpoint {}", req.path);

        json body, response;
        // Store path to saved files for easy rollback if db process fails
        json files_to_save{};
        httplib::MultipartFormDataItems files;

        if (req.is_multipart_form_data())
        {
            // Handle file upload using content receiver pattern
            reader(
                [&](const httplib::MultipartFormData& file) -> bool
                {
                    files.push_back(file);
                    return true;
                },
                [&](const char* data, const size_t data_length) -> bool
                {
                    files.back().content.append(data, data_length);
                    return true;
                });

            // Process uploaded files and form fields
            for (const auto& file : files)
            {
                if (!file.filename.empty())
                {
                    // Ensure field is of file type
                    auto it = std::ranges::find_if(m_fields, [&file](const json& schema_field)
                    {
                        // Check whether the schema name matches the file field name
                        return schema_field.at("name").get<std::string>() == file.name;
                    });

                    // Ensure field being
                    if (it == m_fields.end())
                    {
                        response["status"] = 400;
                        response["error"] = std::format("Unknown field `{}` for file type upload!", file.name);
                        response["data"] = json::object();

                        res.set_content(response.dump(), "application/json");
                        res.status = 400;
                        return;
                    }

                    // Ensure field is of `file|files` type.
                    if (!(it->at("type").get<std::string>() == "file" || it->at("type").get<std::string>() == "files"))
                    {
                        response["status"] = 400;
                        response["error"] = std::format("Field `{}` is not of type `file` or `files`!", file.name);
                        response["data"] = json::object();

                        res.set_content(response.dump(), "application/json");
                        res.status = 400;
                        return;
                    }

                    // Handle file upload
                    const auto dir = MantisApp::instance().files().dirPath(m_tableName, true);
                    const auto new_filename = std::format("{}_{}", generateShortId(8), file.filename);
                    std::string filepath = (fs::path(dir) / new_filename).string();

                    json f;
                    f["filename"] = new_filename;
                    f["path"] = filepath;
                    files_to_save[file.name] = f;

                    // Add file info to JSON body object
                    body[file.name] = new_filename;
                }
                else
                {
                    // This is a regular form field, treat as JSON data
                    try
                    {
                        body[file.name] = json::parse(file.content);
                    }
                    catch (...)
                    {
                        // If not valid JSON, store as string
                        body[file.name] = file.content;
                    }
                }
            }
        }
        else
        {
            // Handle JSON/regular body
            std::string body_str;
            reader([&](const char* data, const size_t data_length) -> bool
            {
                body_str.append(data, data_length);
                return true;
            });

            // Parse request body to JSON Object, return an error if it fails
            try { body = json::parse(body_str); }
            catch (const std::exception& e)
            {
                response["status"] = 400;
                response["error"] = e.what();
                response["data"] = json::object();

                res.set_content(response.dump(), "application/json");
                res.status = 400;
                return;
            }
        }

        // Validate JSON body, return any validation errors encountered
        if (const auto resp = validateRequestBody(body))
        {
            response["status"] = resp.value().value("status", 500);
            response["error"] = resp.value().value("error", "");
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = resp.value().value("status", 500);

            Log::critical("Error Validating Field: {}", resp.value().value("error", ""));
            return;
        };

        for (const auto& file : files)
        {
            const auto filepath = files_to_save[file.name]["path"].get<std::string>();
            Log::trace("Creating new file {}", filepath);
            if (std::ofstream ofs(filepath, std::ios::binary); ofs.is_open())
            {
                ofs.write(file.content.data(), file.content.size());
                ofs.close();
            }
            else
            {
                response["status"] = 500;
                response["error"] = "Failed to save file: " + file.filename;
                response["data"] = json::object();

                res.status = 500;
                res.set_content(response.dump(), "application/json");
                return;
            }
        }

        // Try creating the record, if it checkMinValueFuncs, return the error
        auto respObj = create(body, json{});
        if (!respObj.value("error", "").empty())
        {
            int status = respObj.value("status", 500);
            response["status"] = status;
            response["error"] = respObj.at("error").get<std::string>();
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = status;

            for (const auto& file : files)
            {
                const auto filename = files_to_save[file.name]["filename"].get<std::string>();
                if (!MantisApp::instance().files().removeFile(m_tableName, filename))
                {
                    Log::warn("Could not delete: `{}`", filename);
                }
            }

            Log::critical("Failed to create record, reason: {}", respObj.dump());
            return;
        }

        // Get the data, for auth types, redact the password information
        // it's not useful data to return in the response despite being hashed
        auto record = respObj.at("data");
        Log::trace("Record creation successful: {}", record.dump());

        // For auth types, remove the password field from the response
        if (m_tableType == "auth" && record.contains("password"))
        {
            record.erase("password");
        }

        // Return the added record + the system generated fields
        response["status"] = 201;
        response["error"] = "";
        response["data"] = record;

        res.status = 201;
        res.set_content(response.dump(), "application/json");
    }

    void TableUnit::updateRecord(const Request& req, Response& res, const ContentReader& reader, Context& ctx)
    {
        Log::trace("Updating record, endpoint {}", req.path);

        json body, response;
        // Extract request ID and check that it's not empty
        const auto id = req.path_params.at("id");

        // For empty IDs, return 400, BAD REQUEST
        if (id.empty())
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Record ID is required!";

            res.status = 400;
            res.set_content(response.dump(), "application/json");
            return;
        }

        // TODO handle file updates
        // Store path to saved files for easy rollback if db process fails
        json files_to_save{};
        httplib::MultipartFormDataItems files;

        if (req.is_multipart_form_data())
        {
            // Handle file upload using content receiver pattern
            reader(
                [&](const httplib::MultipartFormData& file) -> bool
                {
                    files.push_back(file);
                    return true;
                },
                [&](const char* data, const size_t data_length) -> bool
                {
                    files.back().content.append(data, data_length);
                    return true;
                });

            // Process uploaded files and form fields
            for (const auto& file : files)
            {
                if (!file.filename.empty())
                {
                    // Handle file upload
                    const auto dir = MantisApp::instance().files().dirPath(m_tableName, true);
                    const auto new_filename = std::format("{}_{}", generateShortId(8), file.filename);
                    std::string filepath = (fs::path(dir) / new_filename).string();

                    // TODO ensure field type is of `file` type
                    json f;
                    f["filename"] = new_filename;
                    f["path"] = dir;
                    files_to_save[file.name] = f;

                    // Add file info to JSON body object
                    body[file.name] = new_filename;
                }
                else
                {
                    // This is a regular form field, treat as JSON data
                    try
                    {
                        body[file.name] = json::parse(file.content);
                    }
                    catch (...)
                    {
                        // If not valid JSON, store as string
                        body[file.name] = file.content;
                    }
                }
            }
        }
        else
        {
            // Handle JSON/regular body
            std::string body_str;
            reader([&](const char* data, const size_t data_length) -> bool
            {
                body_str.append(data, data_length);
                return true;
            });

            // Parse request body to JSON Object, return an error if it fails
            try { body = json::parse(body_str); }
            catch (const std::exception& e)
            {
                response["status"] = 400;
                response["error"] = e.what();
                response["data"] = json::object();

                res.set_content(response.dump(), "application/json");
                res.status = 400;
                return;
            }
        }

        // Check that record exists before we continue ...
        if (!recordExists(id))
        {
            response["status"] = 404;
            response["data"] = json::object();
            response["error"] = "Record with id " + id + " was not found.";

            res.status = 404;
            res.set_content(response.dump(), "application/json");
            return;
        }

        // Validate JSON body, return any validation errors encountered
        if (const auto resp = validateUpdateRequestBody(body))
        {
            response["status"] = resp.value().value("status", 500);
            response["error"] = resp.value().value("error", "");
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = resp.value().value("status", 500);

            Log::critical("Error Validating Field: {}", resp.value().value("error", ""));
            return;
        };

        // Try creating the record, if it checkMinValueFuncs, return the error
        auto respObj = update(id, body, json{});
        if (!respObj.value("error", "").empty())
        {
            int status = respObj.value("status", 500);
            response["status"] = status;
            response["error"] = respObj.at("error").get<std::string>();
            response["data"] = json::object();

            res.set_content(response.dump(), "application/json");
            res.status = status;

            Log::critical("Failed to update record, id = {}, reason: {}", id, respObj.dump());
            return;
        }

        // Get the data, for auth types, redact the password information
        // it's not useful data to return in the response despite being hashed
        auto record = respObj.at("data");
        Log::trace("Record update successful: {}", record.dump());

        // For auth types, remove the password field from the response
        if (m_tableType == "auth" && record.contains("password"))
        {
            record.erase("password");
        }

        // Return the added record + the system generated fields
        response["status"] = 200;
        response["error"] = "";
        response["data"] = record;

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    }

    void TableUnit::deleteRecord(const Request& req, Response& res, Context& ctx)
    {
        Log::trace("Deleting record, endpoint {}", req.path);

        // Extract request ID and check that it's not empty
        const auto id = req.path_params.at("id");

        // For empty IDs, return 400, BAD REQUEST
        if (id.empty())
        {
            json response;
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Record ID is required!";

            res.status = 400;
            res.set_content(response.dump(), "application/json");
            return;
        }

        // The delete record may throw an error, lets catch it here and return
        // appropriately.
        try
        {
            [[maybe_unused]] const auto resp = remove(id, json::object());

            // If all went well, lets return Status OK response
            // no need of a body here
            res.status = 204;
            res.set_content("", "application/json");
        }
        catch (const std::exception& e)
        {
            json response;
            response["status"] = 404;
            response["data"] = json::object();
            response["error"] = e.what();

            res.status = 404;
            res.set_content(response.dump(), "application/json");
        }
    }
}
