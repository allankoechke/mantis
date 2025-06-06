//
// Created by allan on 13/05/2025.
//

#ifndef TABLES_H
#define TABLES_H

#include <memory>
#include <nlohmann/json.hpp>

extern "C" {
    #include <l8w8jwt/encode.h>
}

#include "models.h"
#include "../http.h"
#include "../../app/app.h"
#include "../crud/basecrud.h"

namespace mantis
{
    using json = nlohmann::json;

    using TableValue = std::variant<
        std::monostate,
        std::string,
        double,
        std::tm,
        int8_t, uint8_t,
        int16_t, uint16_t,
        int32_t, uint32_t,
        int64_t, uint64_t,
        bool,
        json,
        BLOB
    >;

    /**
     *
     */
    class TableUnit : public CrudInterface<json>
    {
    public:
        explicit TableUnit(MantisApp* app,
                           std::string tableName,
                           std::string tableId,
                           std::string tableType = "base");
        explicit TableUnit(MantisApp* app,
                           const json& schema = json::object());

        virtual ~TableUnit() = default;

        void setRouteDisplayName(const std::string& routeName);
        virtual bool setupRoutes();

        virtual void fetchRecord(const Request& req, Response& res, Context& ctx);
        virtual void fetchRecords(const Request& req, Response& res, Context& ctx);
        virtual void createRecord(const Request& req, Response& res, Context& ctx);
        virtual void updateRecord(const Request& req, Response& res, Context& ctx);
        virtual void deleteRecord(const Request& req, Response& res, Context& ctx);

        void authWithEmailAndPassword(const Request& req, Response& res, Context& ctx) const;
        void resetPassword(const Request& req, Response& res, Context& ctx);

        // Middleware
        static bool getAuthToken(const Request& req, Response& res, Context& ctx);
        static bool hasAccess(const Request& req, Response& res, Context& ctx);

        // Getters
        std::string tableName();
        void setTableName(const std::string& name);;

        std::string tableId();
        void setTableId(const std::string& id);

        std::string tableType();
        void fromJson(const json& j);

        std::vector<json> fields() const;
        void setFields(const std::vector<json>& fields);

        bool isSystem() const;
        void setIsSystemTable(bool isSystemTable);;

        // Store the rules cached
        Rule listRule();
        void setListRule(const Rule& rule);;

        Rule getRule();
        void setGetRule(const Rule& rule);;

        Rule addRule();
        void setAddRule(const Rule& rule);;

        Rule updateRule();
        void setUpdateRule(const Rule& rule);

        Rule deleteRule();
        void setDeleteRule(const Rule& rule);

        static std::string generateTableId(const std::string& tablename);

        // CRUD endpoints
        // Create/read/list/update/delete record(s), use opts to config optional params
        json create(const json& entity, const json& opts) override;
        std::optional<json> read(const std::string& id, const json& opts) override;
        json update(const std::string& id, const json& entity, const json& opts) override;
        bool remove(const std::string& id, const json& opts) override;
        std::vector<json> list(const json& opts) override;

        // Helper methods
        std::string getColTypeFromName(const std::string& col) const;;
        json parseDbRowToJson(const soci::row& row) const;
        std::optional<json> validateRequestBody(const json& body) const;
        std::optional<json> validateUpdateRequestBody(const json& body) const;
        static TableValue getTypedValue(const json& row, const std::string& colName, const std::string& type);
        bool recordExists(const std::string& id) const;
        std::optional<json> findFieldByKey(const string& key) const;

        // Token Methods
        std::string createJWT(const std::string& user_id, const std::string& user_table) {
            char* jwt = nullptr;
            size_t jwt_length = 0;
            json res{{"error", ""}, {"token", ""}};

            // Initialize encoding parameters
            struct l8w8jwt_encoding_params params;
            l8w8jwt_encoding_params_init(&params);

            // Set algorithm (using HS256 for simplicity)
            params.alg = L8W8JWT_ALG_HS256;

            // Set expiration to 24 hours from now (86400 seconds)
            params.iat = l8w8jwt_time(nullptr);
            params.exp = l8w8jwt_time(nullptr) + 86400;

            const std::string secretKey = m_app->jwtSecretKey();
            // Set secret key
            params.secret_key = (unsigned char*)secretKey.c_str();
            params.secret_key_length = secretKey.length();

            const auto id = const_cast<char*>(user_id.c_str());
            const auto table = const_cast<char*>(user_table.c_str());

            // Create additional payload claims for 'id' and 'table'
            struct l8w8jwt_claim additional_claims[2];
            additional_claims[0] = {
                .key = "id",
                .key_length = 2,
                .value = id,
                .value_length = user_id.length(),
                .type = L8W8JWT_CLAIM_TYPE_STRING
            };
            additional_claims[1] = {
                .key = "table",
                .key_length = 5,
                .value = table,
                .value_length = user_table.length(),
                .type = L8W8JWT_CLAIM_TYPE_STRING
            };

            params.additional_payload_claims = additional_claims;
            params.additional_payload_claims_count = 2;

            // Set output parameters
            params.out = &jwt;
            params.out_length = &jwt_length;

            // Encode the JWT
            if (const int result = l8w8jwt_encode(&params); result == L8W8JWT_SUCCESS && jwt != nullptr) {
                std::string token(jwt);
                l8w8jwt_free(jwt);  // Always free the allocated memory
                res["token"] = token;
                return res;
            }

            // Handle encoding failure
            if (jwt != nullptr) {
                l8w8jwt_free(jwt);
                res["error"] = "JWT Token Encoding Error";
                return res;
            }

            res["error"] = "Could not provision an access token!";
            return res; // Return empty string on failure
        }

    protected:
        std::unique_ptr<MantisApp> m_app;
        std::string m_tableName;
        std::string m_tableId;
        std::string m_tableType;
        std::string m_routeName;
        bool m_isSystem = false;
        std::vector<json> m_fields = {};

        // Store the rules cached
        Rule m_listRule;
        Rule m_getRule;
        Rule m_addRule;
        Rule m_updateRule;
        Rule m_deleteRule;
    };
}

#endif //TABLES_H
