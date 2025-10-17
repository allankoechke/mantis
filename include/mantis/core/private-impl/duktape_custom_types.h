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

    /**
     * @brief A wrapper class around `httplib::Request` offering a
     * consistent API and allowing for easy wrapper methods compatible
     * with `Duktape` API requirements for scripting.
     *
     * Additionally, `MantisRequest` adds a context object for storing
     * some `key`-`value` data for sharing across middlewares and
     * request handlers.
     */
    class MantisRequest
    {
        const httplib::Request& m_req;
        Context m_ctx;

        const std::string __class_name__ = "mantis::MantisRequest";

    public:
        /**
         * @brief Wrapper class around the httplib Request object and
         * our context library.
         *
         * @param _req httplib::Request& object
         */
        explicit MantisRequest(const httplib::Request& _req);

        // Add getters/setters for public members
        std::string getMethod() const;
        void setMethod(const std::string& m) const;

        std::string getPath() const;
        void setPath(const std::string& p) const;

        std::string getBody() const;
        void setBody(const std::string& b) const;

        bool hasHeader(const std::string& key) const;
        std::string getHeaderValue(const std::string& key, const char* def, size_t id) const;

        size_t getHeaderValueU64(const std::string& key, size_t def, size_t id) const;
        size_t getHeaderValueCount(const std::string& key) const;

        void setHeader(const std::string& key, const std::string& val);

        bool hasTrailer(const std::string& key) const;
        std::string getTrailerValue(const std::string& key, size_t id) const;

        size_t getTrailerValueCount(const std::string& key) const;

        bool hasQueryParam(const std::string& key) const;
        std::string getQueryParamValue(const std::string& key) const;
        std::string getQueryParamValue(const std::string& key, size_t id) const;
        size_t getQueryParamValueCount(const std::string& key) const;

        bool hasPathParam(const std::string& key) const;
        std::string getPathParamValue(const std::string& key) const;
        std::string getPathParamValue(const std::string& key, size_t id) const;
        size_t getPathParamValueCount(const std::string& key) const;
        bool isMultipartFormData() const;
        static void registerDuktapeMethods();

    private:
        // Context Methods for setting and getting context values
        bool hasKey(const std::string& key) const;
        DukValue get(const std::string& key);
        DukValue getOr(const std::string& key, const DukValue& default_value);
        void set(const std::string& key, const DukValue& value);
    };

    /**
     * @brief A wrapper class around `httplib::Response` offering a
     * consistent API and allowing for easy wrapper methods compatible
     * with `Duktape` API requirements for scripting.
     */
    class MantisResponse
    {
        httplib::Response& m_res;

        const std::string __class_name__ = "mantis::MantisResponse";

    public:
        explicit MantisResponse(httplib::Response& _resp);
        ~MantisResponse() = default;

        ///> Get Response Status Code
        [[nodiscard]] int getStatus() const;
        ///> Set Response Status Code
        void setStatus(int s);

        [[nodiscard]] std::string getVersion() const;
        void setVersion(const std::string& b);

        ///> Get Response Body
        [[nodiscard]] std::string getBody() const;
        ///> Set Response Body
        void setBody(const std::string& b);

        ///> Get Response redirect location
        [[nodiscard]] std::string getLocation() const;
        ///> Set Response redirect location
        void setLocation(const std::string& b);

        [[nodiscard]] std::string getReason() const;
        void setReason(const std::string& b);

        ///> Check if response has a given header set
        [[nodiscard]] bool hasHeader(const std::string& key) const;
        ///> Get Response header with given header `key`, with an optional default value `def` and index `id` if it's an array
        std::string getHeaderValue(const std::string& key, const char* def = "", size_t id = 0) const;
        ///> Same as @see getHeaderValue() but returns responses as `u64`
        [[nodiscard]] size_t getHeaderValueU64(const std::string& key, size_t def = 0, size_t id = 0) const;
        ///> Get count of the values in a given header entry defined by `key`.
        [[nodiscard]] size_t getHeaderValueCount(const std::string& key) const;
        ///> Set header value with given `key` and `val`
        void setHeader(const std::string& key, const std::string& val) const;

        [[nodiscard]] bool hasTrailer(const std::string& key) const;
        [[nodiscard]] std::string getTrailerValue(const std::string& key, size_t id = 0) const;
        [[nodiscard]] size_t getTrailerValueCount(const std::string& key) const;

        void setRedirect(const std::string& url, int status = httplib::StatusCode::Found_302) const;
        void setContent(const char* s, size_t n, const std::string& content_type) const;
        void setContent(const std::string& s, const std::string& content_type) const;
        void setContent(std::string&& s, const std::string& content_type) const;

        void setFileContent(const std::string& path, const std::string& content_type) const;
        void setFileContent(const std::string& path) const;

        void send(int statusCode, const std::string& data = "", const std::string& content_type= "text/plain") const;
        void sendJson(int statusCode = 200, const json& data = json::object()) const;
        void sendJsonStr(int statusCode, const std::string& data) const; // TODO Switch to DukValue for JSON types
        void sendText(int statusCode = 200, const std::string& data = "") const;
        void sendHtml(int statusCode = 200, const std::string& data = "<p></p>") const;
        void sendEmpty(int statusCode = 204) const;

        static void registerDuktapeMethods();
    };
} // mantis

#endif //MANTISAPP_DUKTAPE_CUSTOM_TYPES_H
