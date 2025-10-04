//
// Created by allan on 30/06/2025.
//

#ifndef SETTINGS_H
#define SETTINGS_H

#include <mantis/core/http.h>
#include <nlohmann/json.hpp>

namespace mantis
{
    using json = nlohmann::json;

    class MantisApp;

    /**
     * @brief Manages application settings
     */
    class SettingsUnit
    {
    public:
        SettingsUnit() = default;

        /**
         * @brief Initialize and set up routes for fetching settings data
         * @return `true` if setting up routes succeeded.
         */
        bool setupRoutes();

        /**
         * @brief Initialize migration, create base data for setting fields
         */
        void migrate();

        /**
         * @brief Evaluate if request is authenticated and has permission to access this route.
         *
         * This route is exclusive to admin login only!
         *
         * @param req HTTP request
         * @param res HTTP response
         * @param ctx HTTP context
         * @return `true` if access is granted, else, `false`
         */
        bool hasAccess([[maybe_unused]] const Request& req, Response& res, Context& ctx) const;

        // Getter sections
        /**
         * @brief Get the current config data instance.
         *
         * @return Config data as a JSON object
         */
        json& configs();

    private:
        /**
         * @brief Fetch config data from database, initializing it to defaults if not existing yet!
         *
         * @return json object having the config, or empty json object.
         */
        json initSettingsConfig();

        /**
         * @brief Called by @see setupRoutes() to initialize routes specific to settings config only!
         */
        void setupConfigRoutes();

        // Cache settings config on create/read/update cycles to reduce database reads
        // may not be that significant though...!
        json m_configs;

        const std::string __class_name__ = "mantis::SettingsUnit";
    };
} // mantis

#endif //SETTINGS_H
