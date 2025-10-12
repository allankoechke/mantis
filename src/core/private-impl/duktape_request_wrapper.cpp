//
// Created by allan on 12/10/2025.
//

#include <mantis/core/private-impl/duktape_custom_types.h>

#include "mantis/app/app.h"

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

    bool MantisRequest::has_param(const std::string& key) const
    {
        return m_req.has_param(key);
    }

    std::string MantisRequest::get_param_value(const std::string& key, size_t id) const
    {
        return m_req.get_param_value(key, id);
    }

    size_t MantisRequest::get_param_value_count(const std::string& key) const
    {
        return m_req.get_param_value_count(key);
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
        dukglue_register_method(ctx, &MantisRequest::has_header, "hasHeader");
        dukglue_register_method(ctx, &MantisRequest::get_header_value, "getHeaderValue");
        dukglue_register_method(ctx, &MantisRequest::get_header_value_u64, "getHeaderValueU64");
        dukglue_register_method(ctx, &MantisRequest::get_header_value_count, "getHeaderValueCount");
        dukglue_register_method(ctx, &MantisRequest::set_header, "setHeader");

        dukglue_register_method(ctx, &MantisRequest::has_trailer, "hasTrailer");
        dukglue_register_method(ctx, &MantisRequest::get_trailer_value, "getTrailerValue");
        dukglue_register_method(ctx, &MantisRequest::get_trailer_value_count, "getTrailerValueCount");

        dukglue_register_method(ctx, &MantisRequest::has_param, "hasParam");
        dukglue_register_method(ctx, &MantisRequest::get_param_value, "getParamValue");
        dukglue_register_method(ctx, &MantisRequest::get_param_value_count, "getParamValueCount");

        dukglue_register_method(ctx, &MantisRequest::is_multipart_form_data, "isMultipartFormData");

        dukglue_register_property(ctx, &MantisRequest::get_body, nullptr, "body");
        dukglue_register_property(ctx, &MantisRequest::get_method, nullptr, "method");
        dukglue_register_property(ctx, &MantisRequest::get_path, nullptr, "path");
    }
}
