//
// Created by allan on 18/10/2025.
//

#include <mantis/core/context_store.h>
#include <mantis/app/app.h>
#include <dukglue/dukglue.h>
#include <mantis/core/logging.h>

namespace mantis
{
    void ContextStore::dump()
    {
        for (const auto& [key, value] : data)
        {
            const auto i = "ContextStore::Dump";
            if (value.type() == typeid(std::string))
            {
                Log::debug("{} - {}: {}", i, key, std::any_cast<std::string>(value));
            }
            else if (value.type() == typeid(const char*))
            {
                Log::debug("{} - {}: {}", i, key, std::any_cast<const char*>(value));
            }
            else if (value.type() == typeid(int))
            {
                Log::debug("{} - {}: {}", i, key, std::any_cast<int>(value));
            }
            else if (value.type() == typeid(double))
            {
                Log::debug("{} - {}: {}", i, key, std::any_cast<double>(value));
            }
            else if (value.type() == typeid(float))
            {
                Log::debug("{} - {}: {}", i, key, std::any_cast<float>(value));
            }
            else if (value.type() == typeid(bool))
            {
                Log::debug("{} - {}: {}", i, key, (std::any_cast<bool>(value) ? "true" : "false"));
            }
            else if (value.type() == typeid(json))
            {
                Log::debug("{} - {}: {}", i, key, std::any_cast<json>(value).dump());
            }
            else
            {
                Log::debug("{} - {}: {}", i, key, "<Unknown Type>");
            }
        }
    }

    bool ContextStore::hasKey(const std::string& key) const
    {
        return data.contains(key);
    }

    DukValue ContextStore::get_duk(const std::string& key)
    {
        const auto it = data.find(key);

        // If no item was found ...
        if (it == data.end())
            return {}; // undefined

        // Convert std::any to DukValue based on stored type
        const std::any& value = it->second;

        const auto ctx = MantisApp::instance().ctx();

        if (value.type() == typeid(int))
        {
            dukglue_push(ctx, std::any_cast<int>(value));
        }
        else if (value.type() == typeid(double))
        {
            dukglue_push(ctx, std::any_cast<double>(value));
        }
        else if (value.type() == typeid(std::string))
        {
            dukglue_push(ctx, std::any_cast<std::string>(value));
        }
        else if (value.type() == typeid(bool))
        {
            dukglue_push(ctx, std::any_cast<bool>(value));
        }
        else if (value.type() == typeid(nlohmann::json))
        {
            const auto& json_obj = std::any_cast<nlohmann::json>(value);
            const std::string json_str = json_obj.dump();

            // Parse JSON string into JavaScript object
            duk_push_string(ctx, json_str.c_str());
            duk_json_decode(ctx, -1);
            return DukValue::take_from_stack(ctx);
        }
        else
        {
            // Unsupported type - throw error
            // Maybe supported later in future
            Log::warn("Unsupported type stored for key `{}`", key);
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "Unsupported type stored for key '%s'", key.c_str());
        }

        return DukValue::take_from_stack(ctx);
    }

    DukValue ContextStore::getOr_duk(const std::string& key, DukValue default_value)
    {
        auto val = get_duk(key);
        if (val == DukValue{})
        {
            return default_value;
        }
        return val;
    }

    void ContextStore::set_duk(const std::string& key, const DukValue& value)
    {
        const auto ctx = MantisApp::instance().ctx();

        // Convert DukValue to std::any based on JavaScript type
        switch (value.type())
        {
        case DukValue::NUMBER:
            data[key] = value.as_double();
            break;
        case DukValue::STRING:
            data[key] = value.as_string();
            break;
        case DukValue::BOOLEAN:
            data[key] = value.as_bool();
            break;
        case DukValue::NULLREF:
        case DukValue::UNDEFINED:
            data.erase(key);
            break;
        case DukValue::OBJECT:
            {
                // Convert JavaScript object to JSON
                value.push();
                const char* json_str = duk_json_encode(ctx, -1);
                nlohmann::json json_obj = nlohmann::json::parse(json_str);
                duk_pop(ctx);

                data[key] = json_obj;
                break;
            }
        default:
            {
                // Unsupported type - throw error
                Log::warn("Unsupported type stored for key `{}`", key);
                duk_error(ctx, DUK_ERR_TYPE_ERROR, "Unsupported type stored for key '%s'", key.c_str());
            }
        }
    }
} // mantis
