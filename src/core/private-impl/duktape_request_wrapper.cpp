//
// Created by allan on 12/10/2025.
//

#include <mantis/core/private-impl/duktape_custom_types.h>

#include "mantis/app/app.h"

#define __file__ "duktape_response_wrapper.cpp"

namespace mantis
{
    MantisRequest::MantisRequest(const httplib::Request& _req, Context& _ctx)
        : m_req(_req),
          m_ctx(_ctx)
    {
    }

    std::string MantisRequest::get_method() const
    {
        return m_req.method;
    }

    void MantisRequest::set_method(const std::string& m) const
    {
        // m_req.method = m;
    }

    std::string MantisRequest::get_path() const
    {
        return m_req.path;
    }

    void MantisRequest::set_path(const std::string& p) const
    {
        // m_req.path = p;
    }

    std::string mantis::MantisRequest::get_body() const
    {
        return m_req.body;
    }

    void MantisRequest::set_body(const std::string& b) const
    {
        // m_req.body = b;
    }

    bool MantisRequest::has_header(const std::string& key) const
    {
        return m_req.has_header(key);
    }

    std::string MantisRequest::get_header_value(const std::string& key, const char* def, size_t id) const
    {
        return m_req.get_header_value(key, def, id);
    }

    size_t MantisRequest::get_header_value_u64(const std::string& key, size_t def, size_t id) const
    {
        return m_req.get_header_value_u64(key, def, id);
    }

    size_t MantisRequest::get_header_value_count(const std::string& key) const
    {
        return m_req.get_header_value_count(key);
    }

    void MantisRequest::set_header(const std::string& key, const std::string& val)
    {
        // m_req.set_header(key, val);
    }

    bool MantisRequest::has_trailer(const std::string& key) const
    {
        return m_req.has_trailer(key);
    }

    std::string MantisRequest::get_trailer_value(const std::string& key, size_t id) const
    {
        return m_req.get_trailer_value(key, id);
    }

    size_t MantisRequest::get_trailer_value_count(const std::string& key) const
    {
        return m_req.get_trailer_value_count(key);
    }

    bool MantisRequest::has_query_param(const std::string& key) const
    {
        Log::trace("Has Param? {}", m_req.has_param(key));
        return m_req.has_param(key);
    }

    std::string MantisRequest::get_query_param_value(const std::string& key) const
    {
        return m_req.get_param_value(key);
    }

    std::string MantisRequest::get_query_param_value(const std::string& key, size_t id) const
    {
        return m_req.get_param_value(key, id);
    }

    size_t MantisRequest::get_query_param_value_count(const std::string& key) const
    {
        return m_req.get_param_value_count(key);
    }


    bool MantisRequest::has_path_param(const std::string& key) const
    {
        return m_req.path_params.contains(key);
    }

    std::string MantisRequest::get_path_param_value(const std::string& key) const
    {
        if (m_req.path_params.contains(key))
            return m_req.path_params.at(key);
        return "";
    }

    std::string MantisRequest::get_path_param_value(const std::string& key, size_t id) const
    {
        if (m_req.path_params.contains(key))
            return m_req.path_params.at(key);
        return "";
    }

    size_t MantisRequest::get_path_param_value_count(const std::string& key) const
    {
        if (m_req.path_params.contains(key))
            return m_req.path_params.at(key).size();
        return 0;
    }

    bool MantisRequest::is_multipart_form_data() const
    {
        return m_req.is_multipart_form_data();
    }

    void MantisRequest::registerDuktapeMethods()
    {
        // Get Duktape Context
        const auto ctx = MantisApp::instance().ctx();

        // Register Request methods
        // `req.hasHeader("Authorization")` -> return true/false
        dukglue_register_method(ctx, &MantisRequest::has_header, "hasHeader");
        // `req.getHeader("Authorization", "", 0)` -> Return Authorization value or default
        // `req.getHeader("Authorization", "Default Value", 0)` -> Return Authorization value or default
        // `req.getHeader("Some Key", "Default Value", 1)` -> Return 'Some Key' value if exists of index '1' or default
        dukglue_register_method(ctx, &MantisRequest::get_header_value, "getHeader");
        dukglue_register_method(ctx, &MantisRequest::get_header_value_u64, "getHeaderU64");
        // `req.getHeaderCount("key")` -> Count for header values given the header key
        dukglue_register_method(ctx, &MantisRequest::get_header_value_count, "getHeaderCount");
        // req.setHeader("Cow", "Cow Value")
        dukglue_register_method(ctx, &MantisRequest::set_header, "setHeader");

        dukglue_register_method(ctx, &MantisRequest::has_trailer, "hasTrailer");
        dukglue_register_method(ctx, &MantisRequest::get_trailer_value, "getTrailer");
        dukglue_register_method(ctx, &MantisRequest::get_trailer_value_count, "getTrailerCount");

        // `req.hasQueryParam("key")` -> return true/false
        dukglue_register_method(ctx, &MantisRequest::has_query_param, "hasQueryParam");
        // `req.getQueryParam("key")` -> Return header value given the key
        dukglue_register_method(ctx, static_cast<std::string(MantisRequest::*)(const std::string&) const>(&MantisRequest::get_query_param_value), "getQueryParam");
        // `req.getQueryParamCount("key")` -> Return parameter value count
        dukglue_register_method(ctx, &MantisRequest::get_query_param_value_count, "getQueryParamCount");

        // `req.hasPathParam("key")` -> return true/false
        dukglue_register_method(ctx, &MantisRequest::has_path_param, "hasPathParam");
        // `req.getPathParam("key")` -> Return header value given the key
        dukglue_register_method(ctx, static_cast<std::string(MantisRequest::*)(const std::string&) const>(&MantisRequest::get_path_param_value), "getPathParam");
        // `req.getPathParamCount("key")` -> Return parameter value count
        dukglue_register_method(ctx, &MantisRequest::get_path_param_value_count, "getPathParamCount");

        // `req.isMultipartFormData()` -> Return true if request type is of Multipart/FormData
        dukglue_register_method(ctx, &MantisRequest::is_multipart_form_data, "isMultipartFormData");

        // `req.body` -> Get request body data
        dukglue_register_property(ctx, &MantisRequest::get_body, nullptr, "body");
        // `req.method` -> Get request method ('GET', 'POST', ...)
        dukglue_register_property(ctx, &MantisRequest::get_method, nullptr, "method");
        // `req.path` -> Get request path value
        dukglue_register_property(ctx, &MantisRequest::get_path, nullptr, "path");
    }
}
