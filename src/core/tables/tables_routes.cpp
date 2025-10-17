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
            Log::debug("Creating route: [{:>6}] {}", "GET", basePath);
            MantisApp::instance().http().Get(
                basePath,
                [this](MantisRequest& req, MantisResponse& res)-> void
                {
                    fetchRecords(req, res);
                },
                {
                    [](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return getAuthToken(req, res);
                    },
                    [this](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return hasAccess(req, res);
                    }
                }
            );

            // Fetch Single Record
            Log::debug("Creating route: [{:>6}] {}{}", "GET/1", basePath, "/:id");
            MantisApp::instance().http().Get(
                basePath + "/:id",
                [this](MantisRequest& req, MantisResponse& res)-> void
                {
                    fetchRecord(req, res);
                },
                {
                    [](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return getAuthToken(req, res);
                    },
                    [this](MantisRequest& req, MantisResponse& res)-> bool
                    {
                        return hasAccess(req, res);
                    }
                }
            );

            // Add/Update and Delete are not supported in views
            if (m_tableType != "view")
            {
                // Add Record
                Log::debug("Creating route: [{:>6}] {}", "POST", basePath);
                MantisApp::instance().http().Post(
                    basePath, [this](MantisRequest& req, MantisResponse& res, const MantisContentReader& reader)-> void
                    {
                        createRecord(req, res, reader);
                    },
                    {
                        [](MantisRequest& req, MantisResponse& res)-> bool
                        {
                            return getAuthToken(req, res);
                        },
                        [this](MantisRequest& req, MantisResponse& res)-> bool
                        {
                            return hasAccess(req, res);
                        }
                    }
                );

                // Update Record
                Log::debug("Creating route: [{:>6}] {}{}", "PATCH", basePath, "/:id");
                MantisApp::instance().http().Patch(
                    basePath + "/:id",
                    [this](MantisRequest& req, MantisResponse& res, const MantisContentReader& reader)-> void
                    {
                        updateRecord(req, res, reader);
                    },
                    {
                        [](MantisRequest& req, MantisResponse& res)-> bool
                        {
                            return getAuthToken(req, res);
                        },
                        [this](MantisRequest& req, MantisResponse& res)-> bool
                        {
                            return hasAccess(req, res);
                        }
                    }
                );

                // Delete Record
                Log::debug("Creating route: [{:>6}] {}{}", "DELETE", basePath, "/:id");
                MantisApp::instance().http().Delete(
                    basePath + "/:id",
                    [this](MantisRequest& req, MantisResponse& res)-> void
                    {
                        deleteRecord(req, res);
                    },
                    {
                        [](MantisRequest& req, MantisResponse& res)-> bool
                        {
                            return getAuthToken(req, res);
                        },
                        [this](MantisRequest& req, MantisResponse& res)-> bool
                        {
                            return hasAccess(req, res);
                        }
                    }
                );
            }

            // Add auth endpoints for login
            if (m_tableType == "auth")
            {
                // Add Record
                Log::debug("Creating route: [{:>6}] {}/auth-with-password", "POST", basePath);
                MantisApp::instance().http().Post(
                    basePath + "/auth-with-password",
                    [this](MantisRequest& req, MantisResponse& res) -> void
                    {
                        authWithEmailAndPassword(req, res);
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

    void TableUnit::fetchRecord(MantisRequest& req, MantisResponse& res)
    {
        TRACE_CLASS_METHOD()

        // Extract request ID and check that it's not empty
        const auto id = req.getPathParamValue("id");

        json response;
        if (id.empty())
        {
            response["status"] = 400;
            response["error"] = "Record ID is required";
            response["data"] = json::object();

            res.sendJson(400, response);
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

                res.sendJson(200, response);
                return;
            }

            response["status"] = 404;
            response["error"] = "Item Not Found";
            response["data"] = json::object();

            res.sendJson(404, response);
        }

        // For any server errors, send it back to the client
        // Capture std::exception, and a catch-all block as well
        catch (const std::exception& e)
        {
            response["status"] = 500;
            response["error"] = e.what();
            response["data"] = json::object();

            res.sendJson(500, response);
        }

        catch (...)
        {
            response["status"] = 500;
            response["error"] = "Unknown Error";
            response["data"] = json::object();

            res.sendJson(500, response);
        }
    }

    void TableUnit::fetchRecords(MantisRequest& req, MantisResponse& res)
    {
        TRACE_CLASS_METHOD()

        // Pagination req params
        json pagination;
        pagination["perPage"] = 100; // Number of records per page
        pagination["pageIndex"] = 1; // Current page, 1-index based
        pagination["pageCount"] = -1; // Total pages
        pagination["recordCount"] = 0; // Total records
        pagination["countPages"] = true; // Whether to calculate number of pages

        if (req.hasQueryParam("perPage") && !req.getQueryParamValue("perPage").empty())
            pagination["perPage"] = std::stoi(req.getQueryParamValue("perPage"));

        if (req.hasQueryParam("pageIndex") && !req.getQueryParamValue("pageIndex").empty())
            pagination["pageIndex"] = std::stoi(req.getQueryParamValue("pageIndex"));

        if (req.hasQueryParam("countPages") && !req.getQueryParamValue("countPages").empty())
        {
            const std::string value = req.getQueryParamValue("countPages");
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

                res.sendJson(400, response);
            }

            response["data"] = resp["data"];
            response["pagination"] = resp["pagination"];
            response["status"] = 200;
            response["error"] = "";

            res.sendJson(200, response);
        }

        catch (const std::exception& e)
        {
            response["data"] = json::array();
            response["status"] = 500;
            response["error"] = e.what();

            res.sendJson(500, response);
        }

        catch (...)
        {
            response["data"] = json::array();
            response["status"] = 500;
            response["error"] = "Internal Server Error";

            res.sendJson(500, response);
        }
    }

    void TableUnit::createRecord(MantisRequest& req, MantisResponse& res, const MantisContentReader& reader)
    {
        TRACE_CLASS_METHOD()

        json body, response;
        // Store path to saved files for easy rollback if db process fails
        json files_to_save{};
        std::vector<httplib::FormData> files;

        if (req.isMultipartFormData())
        {
            // Handle file upload using content receiver pattern
            reader(
                [&](const httplib::FormData& file) -> bool
                {
                    files.push_back(file);
                    return true;
                },
                [&](const char* data, const size_t data_length) -> bool
                {
                    files.back().content.append(data, data_length);
                    return true;
                }
            );

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

                        res.sendJson(400, response);
                        return;
                    }

                    // Ensure field is of `file|files` type.
                    if (!(it->at("type").get<std::string>() == "file" || it->at("type").get<std::string>() == "files"))
                    {
                        response["status"] = 400;
                        response["error"] = std::format("Field `{}` is not of type `file` or `files`!", file.name);
                        response["data"] = json::object();

                        res.sendJson(400, response);
                        return;
                    }

                    // Handle file upload
                    const auto dir = MantisApp::instance().files().dirPath(m_tableName, true);
                    const auto new_filename = sanitizeFilename(file.filename);

                    // Create filepath for writing file contents
                    std::string filepath = (fs::path(dir) / new_filename).string();

                    // File record to be saved
                    json file_record;
                    file_record["filename"] = new_filename;
                    file_record["path"] = filepath; // Path on disk to write the file
                    file_record["name"] = file.name; // Original file name as passed by the user
                    file_record["hash"] = MantisApp::instance().http().hashMultipartMetadata(file);

                    if (it->at("type").get<std::string>() == "file")
                    {
                        // For `file` type
                        files_to_save[file.name] = file_record;

                        // Add to the req body
                        body[file.name] = new_filename;
                    }
                    else
                    {
                        try
                        {
                            // Should be a JSON array, lets construct that if necessary
                            if (!body.contains(file.name)) body[file.name] = nullptr;
                            if (!files_to_save.contains(file.name)) files_to_save[file.name] = nullptr;

                            body[file.name].push_back(new_filename);
                            files_to_save[file.name].push_back(file_record);
                        }
                        catch (std::exception& e)
                        {
                            Log::critical("Error Parsing Files: {}", e.what());

                            response["status"] = 500;
                            response["error"] = e.what();
                            response["data"] = json::object();

                            res.sendJson(500, response);
                            return;
                        }
                    }
                }
                else
                {
                    // This is a regular form field, treat as JSON data
                    try
                    {
                        auto it = std::ranges::find_if(m_fields, [&file](const json& schema_field)
                        {
                            // Check whether the schema name matches the file field name
                            return schema_field.at("name").get<std::string>() == file.name;
                        });

                        if (it != m_fields.end())
                        {
                            try
                            {
                                // Local catch block for JSON parsing errors
                                const auto& type = it->at("type").get<std::string>();

                                // For file types, append the file list to any existing array if any or
                                // parse the array correctly to an array of data
                                if (type == "files")
                                {
                                    auto data = trim(file.content).empty() ? nullptr : json::parse(file.content);
                                    if (!data.is_array() && !data.is_null())
                                    {
                                        response["status"] = 400;
                                        response["data"] = json::object();
                                        response["error"] = std::format("Error parsing field `{}`, expected an array!",
                                                                        file.name);

                                        res.sendJson(400, response);
                                        return;
                                    }

                                    // Create empty field if it does not exist yet
                                    if (!body.contains(file.name)) body[file.name] = nullptr;

                                    // For empty/null values, just continue
                                    if (data == nullptr) continue;

                                    // Append data content to the body field
                                    for (const auto& d : data) body[file.name].push_back(d);
                                }

                                else
                                {
                                    // For all other input types, simply add the data to the respective field.
                                    // Overwrites any existing data
                                    auto v = getValueFromType(type, file.content)["value"];
                                    body[file.name] = v;
                                }
                            }
                            catch (std::exception& e)
                            {
                                response["status"] = 500;
                                response["data"] = json::object();
                                response["error"] = e.what();

                                res.sendJson(500, response);
                                Log::critical("Error parsing field data: {}\n\t- Data: {}: {}", e.what(), file.name,
                                              file.content);
                                return;
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        response["status"] = 500;
                        response["data"] = json::object();
                        response["error"] = e.what();

                        res.sendJson(500, response);
                        Log::critical("Error parsing field data: {}", e.what());
                        return;
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
            try
            {
                if (body_str.empty()) body = json::object();
                else body = json::parse(body_str);
            }
            catch (const std::exception& e)
            {
                response["status"] = 400;
                response["error"] = e.what();
                response["data"] = json::object();

                res.sendJson(400, response);
                return;
            }
        }

        // Validate JSON body, return any validation errors encountered
        const auto resp = validateRequestBody(body);
        if (resp.has_value())
        {
            response["status"] = 400;
            response["error"] = resp.value();
            response["data"] = json::object();

            res.sendJson(400, response);
            Log::critical("Error Validating Request Body: {}", resp.value());
            return;
        };

        // For tracking written files, just in case we need to revert changes
        std::vector<std::string> saved_files{};

        for (const auto& file : files)
        {
            // For non-file types, continue
            if (file.filename.empty()) continue;

            const auto file_list = files_to_save[file.name].is_array()
                                       ? files_to_save[file.name]
                                       : json::array({files_to_save[file.name]});

            auto it = std::ranges::find_if(file_list, [&](const json& f)
            {
                return f["hash"].get<std::string>() == MantisApp::instance().http().hashMultipartMetadata(file);
            });

            if (it == file_list.end())
            {
                // Should not happen, but if it does, throw 500 error
                response["status"] = 500;
                response["data"] = json::object();
                response["error"] = "Error writing files, hash mismatch!";

                res.sendJson(500, response);
                return;
            }

            auto& file_record = *it;
            const auto filepath = file_record["path"].get<std::string>();
            if (std::ofstream ofs(filepath, std::ios::binary); ofs.is_open())
            {
                ofs.write(file.content.data(), file.content.size());
                ofs.close();

                // Keep track of written files
                saved_files.push_back(file_record["filename"].get<std::string>());
            }
            else
            {
                response["status"] = 500;
                response["error"] = "Failed to save file: " + file.filename;
                response["data"] = json::object();

                res.sendJson(500, response);

                // Remove any written files
                for (const auto& f : saved_files)
                {
                    [[maybe_unused]]
                        auto _ = MantisApp::instance().files().removeFile(m_tableName, f);
                }

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

            res.sendJson(status, response);

            for (const auto& f : saved_files)
            {
                if (!MantisApp::instance().files().removeFile(m_tableName, f))
                {
                    Log::warn("Could not delete: `{}`", f);
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

        res.sendJson(201, response);
    }

    void TableUnit::updateRecord(MantisRequest& req, MantisResponse& res, const MantisContentReader& reader)
    {
        TRACE_CLASS_METHOD()

        json body, response;
        // Extract request ID and check that it's not empty
        const auto id = req.getPathParamValue("id");

        // For empty IDs, return 400, BAD REQUEST
        if (id.empty())
        {
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Record ID is required!";

            res.sendJson(400, response);
            return;
        }
        // Check that record exists before we continue ...
        if (!recordExists(id))
        {
            response["status"] = 404;
            response["data"] = json::object();
            response["error"] = "Record with id " + id + " was not found.";

            res.sendJson(404, response);
            return;
        }

        // Store path to saved files for easy rollback if db process fails
        json files_to_save{};
        std::vector<httplib::FormData> files;

        if (req.isMultipartFormData())
        {
            // Handle file upload using content receiver pattern
            reader(
                [&](const httplib::FormData& file) -> bool
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

                        res.sendJson(400, response);
                        return;
                    }

                    // Ensure field is of `file|files` type.
                    if (!(it->at("type").get<std::string>() == "file" || it->at("type").get<std::string>() == "files"))
                    {
                        response["status"] = 400;
                        response["error"] = std::format("Field `{}` is not of type `file` or `files`!", file.name);
                        response["data"] = json::object();

                        res.sendJson(400, response);
                        return;
                    }

                    // Handle file upload
                    const auto dir = MantisApp::instance().files().dirPath(m_tableName, true);
                    const auto new_filename = sanitizeFilename(file.filename);

                    // Create filepath for writing file contents
                    std::string filepath = (fs::path(dir) / new_filename).string();

                    // File record to be saved
                    json file_record;
                    file_record["filename"] = new_filename;
                    file_record["path"] = filepath; // Path on disk to write the file
                    file_record["name"] = file.name; // Original file name as passed by the user
                    file_record["hash"] = MantisApp::instance().http().hashMultipartMetadata(file);

                    if (it->at("type").get<std::string>() == "file")
                    {
                        // For `file` type
                        files_to_save[file.name] = file_record;

                        // Add to the req body
                        body[file.name] = new_filename;
                    }
                    else
                    {
                        try
                        {
                            // Should be a JSON array, lets construct that if necessary
                            if (!body.contains(file.name)) body[file.name] = nullptr;
                            if (!files_to_save.contains(file.name)) files_to_save[file.name] = nullptr;

                            body[file.name].push_back(new_filename);
                            files_to_save[file.name].push_back(file_record);
                        }
                        catch (std::exception& e)
                        {
                            Log::critical("Error Parsing Files: {}", e.what());

                            response["status"] = 500;
                            response["error"] = e.what();
                            response["data"] = json::object();

                            res.sendJson(500, response);
                            return;
                        }
                    }
                }
                else
                {
                    // This is a regular form field, treat as JSON data
                    try
                    {
                        auto it = std::ranges::find_if(m_fields, [&file](const json& schema_field)
                        {
                            // Check whether the schema name matches the file field name
                            return schema_field.at("name").get<std::string>() == file.name;
                        });

                        if (it != m_fields.end())
                        {
                            try
                            {
                                // Local catch block for JSON parsing errors
                                const auto& type = it->at("type").get<std::string>();

                                // For file types, append the file list to any existing array if any or
                                // parse the array correctly to an array of data
                                if (type == "files")
                                {
                                    auto data = trim(file.content).empty() ? nullptr : json::parse(file.content);
                                    if (!data.is_array() && !data.is_null())
                                    {
                                        response["status"] = 400;
                                        response["data"] = json::object();
                                        response["error"] = std::format("Error parsing field `{}`, expected an array!",
                                                                        file.name);

                                        res.sendJson(400, response);
                                        return;
                                    }

                                    // Create empty field if it does not exist yet
                                    if (!body.contains(file.name)) body[file.name] = nullptr;

                                    // For empty/null values, just continue
                                    if (data == nullptr) continue;

                                    // Append data content to the body field
                                    for (const auto& d : data) body[file.name].push_back(d);
                                }

                                else
                                {
                                    // For all other input types, simply add the data to the respective field.
                                    // Overwrites any existing data
                                    auto v = getValueFromType(type, file.content)["value"];
                                    std::cout << type << " - " << v << ";" << std::endl;
                                    body[file.name] = v;
                                }
                            }
                            catch (std::exception& e)
                            {
                                response["status"] = 500;
                                response["data"] = json::object();
                                response["error"] = e.what();

                                res.sendJson(500, response);
                                Log::critical("Error parsing field data: {}\n\t- Data: {}: {}", e.what(), file.name,
                                              file.content);
                                return;
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        response["status"] = 500;
                        response["data"] = json::object();
                        response["error"] = e.what();

                        res.sendJson(500, response);
                        Log::critical("Error parsing field data: {}", e.what());
                        return;
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
            try { if (!body_str.empty()) body = json::parse(body_str); }
            catch (const std::exception& e)
            {
                response["status"] = 400;
                response["error"] = e.what();
                response["data"] = json::object();

                res.sendJson(400, response);
                return;
            }
        }

        // Validate JSON body, return any validation errors encountered
        if (const auto resp = validateUpdateRequestBody(body))
        {
            response["status"] = 400;
            response["error"] = resp.value();
            response["data"] = json::object();

            res.sendJson(400, response);
            Log::critical("Error Validating Field: {}", resp.value());
            return;
        };

        // For tracking written files, just in case we need to revert changes
        std::vector<std::string> saved_files{};

        for (const auto& file : files)
        {
            // For non-file types, continue
            if (file.filename.empty()) continue;

            const auto file_list = files_to_save[file.name].is_array()
                                       ? files_to_save[file.name]
                                       : json::array({files_to_save[file.name]});

            auto it = std::ranges::find_if(file_list, [&](const json& f)
            {
                return f["hash"].get<std::string>() == MantisApp::instance().http().hashMultipartMetadata(file);
            });

            if (it == file_list.end())
            {
                // Should not happen, but if it does, throw 500 error
                response["status"] = 500;
                response["data"] = json::object();
                response["error"] = "Error writing files, hash mismatch!";

                res.sendJson(500, response);
                return;
            }

            auto& file_record = *it;
            const auto filepath = file_record["path"].get<std::string>();
            if (std::ofstream ofs(filepath, std::ios::binary); ofs.is_open())
            {
                ofs.write(file.content.data(), file.content.size());
                ofs.close();

                // Keep track of written files
                saved_files.push_back(file_record["filename"].get<std::string>());
            }
            else
            {
                response["status"] = 500;
                response["error"] = "Failed to save file: " + file.filename;
                response["data"] = json::object();

                res.sendJson(500, response);

                // Remove any written files
                for (const auto& f : saved_files)
                {
                    if (!MantisApp::instance().files().removeFile(m_tableName, f))
                    {
                        Log::warn("Could not delete: `{}`", f);
                    }
                }

                return;
            }
        }

        // Try creating the record, if it checkMinValueFuncs, return the error
        auto respObj = update(id, body, json{});
        if (!respObj.value("error", "").empty())
        {
            int status = respObj.value("status", 500);
            response["status"] = status;
            response["error"] = respObj.at("error").get<std::string>();
            response["data"] = json::object();

            res.sendJson(status, response);

            for (const auto& f : saved_files)
            {
                if (!MantisApp::instance().files().removeFile(m_tableName, f))
                {
                    Log::warn("Could not delete: `{}`", f);
                }
            }

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

        res.sendJson(200, response);
    }

    void TableUnit::deleteRecord(MantisRequest& req, MantisResponse& res)
    {
        Log::trace("Deleting record, endpoint {}", req.getPath());

        // Extract request ID and check that it's not empty
        const auto id = req.getPathParamValue("id");

        // For empty IDs, return 400, BAD REQUEST
        if (id.empty())
        {
            json response;
            response["status"] = 400;
            response["data"] = json::object();
            response["error"] = "Record ID is required!";

            res.sendJson(400, response);
            return;
        }

        // The delete record may throw an error, lets catch it here and return
        // appropriately.
        try
        {
            [[maybe_unused]] const auto resp = remove(id, json::object());

            // If all went well, lets return Status OK response
            // no need of a body here
            res.sendEmpty();
        }
        catch (const std::exception& e)
        {
            json response;
            response["status"] = 404;
            response["data"] = json::object();
            response["error"] = e.what();

            res.sendJson(404, response);
        }
    }
}
