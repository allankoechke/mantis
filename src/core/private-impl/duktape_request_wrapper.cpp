//
// Created by allan on 12/10/2025.
//

#include <mantis/core/private-impl/duktape_custom_types.h>

#include "mantis/app/app.h"

#define __file__ "duktape_response_wrapper.cpp"

namespace mantis
{
    MantisRequest::MantisRequest(const httplib::Request& _req)
        : m_req(_req),
          m_ctx(Context{})
    {
    }

    std::string MantisRequest::getMethod() const
    {
        return m_req.method;
    }

    void MantisRequest::setMethod(const std::string& m) const
    {
        // m_req.method = m;
    }

    std::string MantisRequest::getPath() const
    {
        return m_req.path;
    }

    void MantisRequest::setPath(const std::string& p) const
    {
        // m_req.path = p;
    }

    std::string mantis::MantisRequest::getBody() const
    {
        return m_req.body;
    }

    void MantisRequest::setBody(const std::string& b) const
    {
        // m_req.body = b;
    }

    bool MantisRequest::hasHeader(const std::string& key) const
    {
        return m_req.has_header(key);
    }

    std::string MantisRequest::getHeaderValue(const std::string& key, const char* def, size_t id) const
    {
        return m_req.get_header_value(key, def, id);
    }

    size_t MantisRequest::getHeaderValueU64(const std::string& key, size_t def, size_t id) const
    {
        return m_req.get_header_value_u64(key, def, id);
    }

    size_t MantisRequest::getHeaderValueCount(const std::string& key) const
    {
        return m_req.get_header_value_count(key);
    }

    void MantisRequest::setHeader(const std::string& key, const std::string& val)
    {
        // m_req.set_header(key, val);
    }

    bool MantisRequest::hasTrailer(const std::string& key) const
    {
        return m_req.has_trailer(key);
    }

    std::string MantisRequest::getTrailerValue(const std::string& key, size_t id) const
    {
        return m_req.get_trailer_value(key, id);
    }

    size_t MantisRequest::getTrailerValueCount(const std::string& key) const
    {
        return m_req.get_trailer_value_count(key);
    }

    bool MantisRequest::hasQueryParam(const std::string& key) const
    {
        Log::trace("Has Param? {}", m_req.has_param(key));
        return m_req.has_param(key);
    }

    std::string MantisRequest::getQueryParamValue(const std::string& key) const
    {
        return m_req.get_param_value(key);
    }

    std::string MantisRequest::getQueryParamValue(const std::string& key, size_t id) const
    {
        return m_req.get_param_value(key, id);
    }

    size_t MantisRequest::getQueryParamValueCount(const std::string& key) const
    {
        return m_req.get_param_value_count(key);
    }


    bool MantisRequest::hasPathParam(const std::string& key) const
    {
        return m_req.path_params.contains(key);
    }

    std::string MantisRequest::getPathParamValue(const std::string& key) const
    {
        if (m_req.path_params.contains(key))
            return m_req.path_params.at(key);

        return "";
    }

    std::string MantisRequest::getPathParamValue(const std::string& key, size_t id) const
    {
        if (m_req.path_params.contains(key))
            return m_req.path_params.at(key);

        return "";
    }

    size_t MantisRequest::getPathParamValueCount(const std::string& key) const
    {
        if (m_req.path_params.contains(key))
            return m_req.path_params.at(key).size();

        return 0;
    }

    bool MantisRequest::isMultipartFormData() const
    {
        return m_req.is_multipart_form_data();
    }

    void MantisRequest::registerDuktapeMethods()
    {
        // Get Duktape Context
        const auto ctx = MantisApp::instance().ctx();

        // Register Request methods
        // `req.hasHeader("Authorization")` -> return true/false
        dukglue_register_method(ctx, &MantisRequest::hasHeader, "hasHeader");
        // `req.getHeader("Authorization", "", 0)` -> Return Authorization value or default
        // `req.getHeader("Authorization", "Default Value", 0)` -> Return Authorization value or default
        // `req.getHeader("Some Key", "Default Value", 1)` -> Return 'Some Key' value if exists of index '1' or default
        dukglue_register_method(ctx, &MantisRequest::getHeaderValue, "getHeader");
        dukglue_register_method(ctx, &MantisRequest::getHeaderValueU64, "getHeaderU64");
        // `req.getHeaderCount("key")` -> Count for header values given the header key
        dukglue_register_method(ctx, &MantisRequest::getHeaderValueCount, "getHeaderCount");
        // req.setHeader("Cow", "Cow Value")
        dukglue_register_method(ctx, &MantisRequest::setHeader, "setHeader");

        dukglue_register_method(ctx, &MantisRequest::hasTrailer, "hasTrailer");
        dukglue_register_method(ctx, &MantisRequest::getTrailerValue, "getTrailer");
        dukglue_register_method(ctx, &MantisRequest::getTrailerValueCount, "getTrailerCount");

        // `req.hasQueryParam("key")` -> return true/false
        dukglue_register_method(ctx, &MantisRequest::hasQueryParam, "hasQueryParam");
        // `req.getQueryParam("key")` -> Return header value given the key
        dukglue_register_method(
            ctx, static_cast<std::string(MantisRequest::*)(const std::string&) const>(&
                MantisRequest::getQueryParamValue), "getQueryParam");
        // `req.getQueryParamCount("key")` -> Return parameter value count
        dukglue_register_method(ctx, &MantisRequest::getQueryParamValueCount, "getQueryParamCount");

        // `req.hasPathParam("key")` -> return true/false
        dukglue_register_method(ctx, &MantisRequest::hasPathParam, "hasPathParam");
        // `req.getPathParam("key")` -> Return header value given the key
        dukglue_register_method(
            ctx, static_cast<std::string(MantisRequest::*)(const std::string&) const>(&
                MantisRequest::getPathParamValue), "getPathParam");
        // `req.getPathParamCount("key")` -> Return parameter value count
        dukglue_register_method(ctx, &MantisRequest::getPathParamValueCount, "getPathParamCount");

        // `req.isMultipartFormData()` -> Return true if request type is of Multipart/FormData
        dukglue_register_method(ctx, &MantisRequest::isMultipartFormData, "isMultipartFormData");

        // `req.body` -> Get request body data
        dukglue_register_property(ctx, &MantisRequest::getBody, nullptr, "body");
        // `req.method` -> Get request method ('GET', 'POST', ...)
        dukglue_register_property(ctx, &MantisRequest::getMethod, nullptr, "method");
        // `req.path` -> Get request path value
        dukglue_register_property(ctx, &MantisRequest::getPath, nullptr, "path");


        // `req.hasKey("key")` -> return true if key is in the context store
        dukglue_register_method(ctx, &MantisRequest::hasKey, "hasKey");
        // `req.set("key", value)` // Store in the context store the given key and value. Note only int, float, double, str
        dukglue_register_method(ctx, &MantisRequest::set, "set");
        // `req.get("key")`
        dukglue_register_method(ctx, &MantisRequest::get, "get");
        // `req.getOr("key", default_value)`
        dukglue_register_method(ctx, &MantisRequest::getOr, "getOr");
    }

    bool MantisRequest::hasKey(const std::string& key) const
    {
        return m_ctx.hasKey(key);
    }

    DukValue MantisRequest::get(const std::string& key)
    {
        return m_ctx.get(key);
    }

    DukValue MantisRequest::getOr(const std::string& key, const DukValue& default_value)
    {
        return m_ctx.getOr(key, default_value);
    }

    void MantisRequest::set(const std::string& key, const DukValue& value)
    {
        m_ctx.set(key, value);
    }
}
