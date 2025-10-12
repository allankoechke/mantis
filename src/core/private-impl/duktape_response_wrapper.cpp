//
// Created by allan on 12/10/2025.
//

#include <mantis/core/private-impl/duktape_custom_types.h>

#include "mantis/app/app.h"

namespace mantis
{
    MantisResponse::MantisResponse(httplib::Response& _resp) : m_res(_resp)
    {
    }

    int MantisResponse::get_status() const
    {
        return m_res.status;
    }

    void MantisResponse::set_status(const int s)
    {
        m_res.status = s;
    }

    std::string MantisResponse::get_version() const
    {
        return m_res.version;
    }

    void MantisResponse::set_version(const std::string& b)
    {
        m_res.version = b;
    }

    std::string MantisResponse::get_body() const
    {
        return m_res.body;
    }

    void MantisResponse::set_body(const std::string& b)
    {
        m_res.body = b;
    }

    std::string MantisResponse::get_location() const
    {
        return m_res.location;
    }

    void MantisResponse::set_location(const std::string& b)
    {
        m_res.location = b;
    }

    std::string MantisResponse::get_reason() const
    {
        return m_res.reason;
    }

    void MantisResponse::set_reason(const std::string& b)
    {
        m_res.reason = b;
    }

    bool MantisResponse::has_header(const std::string& key) const
    {
        return m_res.has_header(key);
    }

    std::string MantisResponse::get_header_value(const std::string& key, const char* def, size_t id) const
    {
        return m_res.get_header_value(key, def, id);
    }

    size_t MantisResponse::get_header_value_u64(const std::string& key, size_t def, size_t id) const
    {
        return m_res.get_header_value_u64(key, def, id);
    }

    size_t MantisResponse::get_header_value_count(const std::string& key) const
    {
        return m_res.get_header_value_count(key);
    }

    void MantisResponse::set_header(const std::string& key, const std::string& val) const
    {
        m_res.set_header(key, val);
    }

    bool MantisResponse::has_trailer(const std::string& key) const
    {
        return m_res.has_trailer(key);
    }

    std::string MantisResponse::get_trailer_value(const std::string& key, size_t id) const
    {
        return m_res.get_trailer_value(key, id);
    }

    size_t MantisResponse::get_trailer_value_count(const std::string& key) const
    {
        return m_res.get_trailer_value_count(key);
    }

    void MantisResponse::set_redirect(const std::string& url, int status) const
    {
        m_res.set_redirect(url, status);
    }

    void MantisResponse::set_content(const char* s, size_t n, const std::string& content_type) const
    {
        m_res.set_content(s, n, content_type);
    }

    void MantisResponse::set_content(const std::string& s, const std::string& content_type) const
    {
        m_res.set_content(s, content_type);
    }

    void MantisResponse::set_content(std::string&& s, const std::string& content_type) const
    {
        m_res.set_content(s, content_type);
    }

    void MantisResponse::set_file_content(const std::string& path, const std::string& content_type) const
    {
        m_res.set_file_content(path, content_type);
    }

    void MantisResponse::set_file_content(const std::string& path) const
    {
        m_res.set_file_content(path);
    }

    void MantisResponse::send(const std::string& data, const std::string& content_type, const int code) const
    {
        m_res.set_content(data, content_type);
        m_res.status = code;
    }

    void MantisResponse::registerDuktapeMethods()
    {
        // Get Duktape context
        const auto ctx = MantisApp::instance().ctx();

        // Register Response methods
        dukglue_register_method(ctx, &MantisResponse::has_header, "hasHeader");
        dukglue_register_method(ctx, &MantisResponse::get_header_value, "headerValue");
        dukglue_register_method(ctx, &MantisResponse::get_header_value_u64, "headerValueU64");
        dukglue_register_method(ctx, &MantisResponse::get_header_value_count, "headerValueCount");
        dukglue_register_method(ctx, &MantisResponse::set_header, "setHeader");

        dukglue_register_method(ctx, &MantisResponse::has_trailer, "hasTrailer");
        dukglue_register_method(ctx, &MantisResponse::get_trailer_value, "trailerValue");
        dukglue_register_method(ctx, &MantisResponse::get_trailer_value_count, "trailerValueCount");

        dukglue_register_method(ctx, &MantisResponse::set_redirect, "setRedirect");
        // dukglue_register_method(ctx, &MantisResponse::set_content, "setContent");
        // dukglue_register_method(ctx, &MantisResponse::set_file_content, "setFileContent");

        dukglue_register_method(ctx, &MantisResponse::send, "send");


        dukglue_register_property(ctx, &MantisResponse::get_body, &MantisResponse::set_body, "body");
        dukglue_register_property(ctx, &MantisResponse::get_status, &MantisResponse::set_status, "status");
        dukglue_register_property(ctx, &MantisResponse::get_version, &MantisResponse::set_version, "version");
        dukglue_register_property(ctx, &MantisResponse::get_location, &MantisResponse::set_location, "location");
        dukglue_register_property(ctx, &MantisResponse::get_reason, &MantisResponse::set_reason, "reason");
    }
}
