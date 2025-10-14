//
// Created by allan on 12/10/2025.
//

#include <mantis/core/private-impl/duktape_custom_types.h>

#include "mantis/app/app.h"

#define __file__ "duktape_response_wrapper.cpp"

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

    void MantisResponse::send(int statusCode = 200, const std::string& data, const std::string& content_type) const
    {
        m_res.set_content(data, content_type);
        m_res.status = statusCode;
    }

    void MantisResponse::send_json_str(const int statusCode, const std::string& data) const
    {
        send(statusCode, data, "application/json");
    }

    void MantisResponse::send_text(const int statusCode, const std::string& data) const
    {
        send(statusCode, data, "text/plain");
    }

    void MantisResponse::send_json(const int statusCode, const json& data) const
    {
        send(statusCode, data.dump(), "application/json");
    }

    void MantisResponse::send_html(const int statusCode, const std::string& data) const
    {
        send(statusCode, data, "application/json");
    }

    void MantisResponse::send_empty(const int statusCode) const
    {
        m_res.set_content(std::string{}, std::string{});
        m_res.status = statusCode;
    }

    void MantisResponse::registerDuktapeMethods()
    {
        // Get Duktape context
        const auto ctx = MantisApp::instance().ctx();

        // Register Response methods
        // `res.hasHeader("Authorization")` -> return true/false
        dukglue_register_method(ctx, &MantisResponse::has_header, "hasHeader");
        // `res.getHeader("Authorization", "", 0)` -> Return Authorization value or default
        // `res.getHeader("Authorization", "Default Value", 0)` -> Return Authorization value or default
        // `res.getHeader("Some Key", "Default Value", 1)` -> Return 'Some Key' value if exists of index '1' or default
        dukglue_register_method(ctx, &MantisResponse::get_header_value, "getHeader");
        dukglue_register_method(ctx, &MantisResponse::get_header_value_u64, "getHeaderU64");
        // `res.getHeaderCount("key")` -> Count for header values given the header key
        dukglue_register_method(ctx, &MantisResponse::get_header_value_count, "getHeaderCount");
        // res.setHeader("Cow", "Cow Value")
        dukglue_register_method(ctx, &MantisResponse::set_header, "setHeader");

        dukglue_register_method(ctx, &MantisResponse::has_trailer, "hasTrailer");
        dukglue_register_method(ctx, &MantisResponse::get_trailer_value, "getTrailer");
        dukglue_register_method(ctx, &MantisResponse::get_trailer_value_count, "getTrailerCount");

        // `res.redirect("http://some-url.com", 302)`
        dukglue_register_method(ctx, &MantisResponse::set_redirect, "redirect");

        // `res.setContent("something here", "text/plain")`
        dukglue_register_method(ctx, static_cast<void(MantisResponse::*)(const std::string&, const std::string&) const>(&MantisResponse::set_content), "setContent");
        // `res.setFileContent("/foo/bar.txt")`
        dukglue_register_method(ctx, static_cast<void(MantisResponse::*)(const std::string&) const>(&MantisResponse::set_file_content), "setFileContent");

        // `res.send(200, "some data here", "text/plain")`
        dukglue_register_method(ctx, &MantisResponse::send, "send");
        // `res.json(200, "{\"a\": 5}")`
        dukglue_register_method(ctx, &MantisResponse::send_json_str, "json");
        // `res.html(200, "<html> ... </html>")`
        dukglue_register_method(ctx, &MantisResponse::send_html, "html");
        // `res.text(200, "some text response")`
        dukglue_register_method(ctx, &MantisResponse::send_text, "text");
        // `res.empty(204)`
        dukglue_register_method(ctx, &MantisResponse::send_empty, "empty");

        // `res.body = "Some Data"`
        // `res.body` -> returns `Some Data`
        dukglue_register_property(ctx, &MantisResponse::get_body, &MantisResponse::set_body, "body");
        // `res.status` (get or set status data)
        dukglue_register_property(ctx, &MantisResponse::get_status, &MantisResponse::set_status, "status");
        // `res.version` (get or set version value)
        dukglue_register_property(ctx, &MantisResponse::get_version, &MantisResponse::set_version, "version");
        // `res.location` (get or set redirect location value)
        dukglue_register_property(ctx, &MantisResponse::get_location, &MantisResponse::set_location, "location");
        // `res.reason` (get or set reason value)
        dukglue_register_property(ctx, &MantisResponse::get_reason, &MantisResponse::set_reason, "reason");
    }
}
