//
// Created by allan on 07/10/2025.
//

#ifndef MANTISAPP_DUKTAPE_CUSTOM_TYPES_H
#define MANTISAPP_DUKTAPE_CUSTOM_TYPES_H

#include <dukglue/dukglue.h>
#include "mantis/core/http.h"

namespace mantis
{
    class DuktapeImpl
    {
    public:
        static duk_ret_t nativeConsoleInfo(duk_context* ctx);

        static duk_ret_t nativeConsoleTrace(duk_context* ctx);

        static duk_ret_t nativeConsoleTable(duk_context* ctx);
    };

    // Subclass httplib::Request
    class MantisRequest
    {
        const httplib::Request& m_req;
        Context& m_ctx;

        const std::string __class_name__ = "mantis::MantisRequest";

    public:
        /**
         * @brief Wrapper class around the httplib Request object and
         * our context library.
         *
         * @param _req httplib::Request& object
         * @param _ctx mantis::Context& object
         */
        MantisRequest(const httplib::Request& _req, Context& _ctx);

        // Add getters/setters for public members
        std::string get_method() const;
        void set_method(const std::string& m) const;

        std::string get_path() const;
        void set_path(const std::string& p) const;

        std::string get_body() const;
        void set_body(const std::string& b) const;

        bool has_header(const std::string& key) const;
        std::string get_header_value(const std::string& key, const char* def, size_t id) const;

        size_t get_header_value_u64(const std::string& key, size_t def, size_t id) const;
        size_t get_header_value_count(const std::string& key) const;

        void set_header(const std::string& key, const std::string& val);

        bool has_trailer(const std::string& key) const;
        std::string get_trailer_value(const std::string& key, size_t id) const;

        size_t get_trailer_value_count(const std::string& key) const;

        bool has_query_param(const std::string& key) const;
        std::string get_query_param_value(const std::string& key) const;
        std::string get_query_param_value(const std::string& key, size_t id) const;
        size_t get_query_param_value_count(const std::string& key) const;

        bool has_path_param(const std::string& key) const;
        std::string get_path_param_value(const std::string& key) const;
        std::string get_path_param_value(const std::string& key, size_t id) const;
        size_t get_path_param_value_count(const std::string& key) const;

        bool is_multipart_form_data() const;

        static void registerDuktapeMethods();
    };

    // Subclass httplib::Response
    class MantisResponse
    {
        httplib::Response& m_res;

        const std::string __class_name__ = "mantis::MantisResponse";

    public:
        MantisResponse(httplib::Response& _resp);
        ~MantisResponse() { }

        int get_status() const;
        void set_status(int s);

        std::string get_version() const;
        void set_version(const std::string& b);

        std::string get_body() const;
        void set_body(const std::string& b);

        std::string get_location() const;
        void set_location(const std::string& b);

        std::string get_reason() const;
        void set_reason(const std::string& b);

        bool has_header(const std::string& key) const;

        std::string get_header_value(const std::string& key, const char* def = "", size_t id = 0) const;
        size_t get_header_value_u64(const std::string& key, size_t def = 0, size_t id = 0) const;

        size_t get_header_value_count(const std::string& key) const;
        void set_header(const std::string& key, const std::string& val) const;

        bool has_trailer(const std::string& key) const;
        std::string get_trailer_value(const std::string& key, size_t id = 0) const;
        size_t get_trailer_value_count(const std::string& key) const;

        void set_redirect(const std::string& url, int status = httplib::StatusCode::Found_302) const;
        void set_content(const char* s, size_t n, const std::string& content_type) const;
        void set_content(const std::string& s, const std::string& content_type) const;
        void set_content(std::string&& s, const std::string& content_type) const;

        void set_file_content(const std::string& path, const std::string& content_type) const;
        void set_file_content(const std::string& path) const;

        void send(int statusCode, const std::string& data = "", const std::string& content_type= "text/plain") const;
        void send_json(int statusCode = 200, const json& data = json::object()) const;
        void send_json_str(int statusCode, const std::string& data) const;
        void send_text(int statusCode = 200, const std::string& data = "") const;
        void send_html(int statusCode = 200, const std::string& data = "<p></p>") const;
        void send_empty(int statusCode = 204) const;

        static void registerDuktapeMethods();
    };
} // mantis

#endif //MANTISAPP_DUKTAPE_CUSTOM_TYPES_H
