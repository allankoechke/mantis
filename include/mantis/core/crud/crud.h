//
// Created by allan on 18/05/2025.
//

#ifndef CRUD_H
#define CRUD_H

#include <optional>
#include <vector>
#include <nlohmann/json.hpp>

namespace mantis
{
    using json = nlohmann::json;

    template <typename T>
    class CrudInterface {
        public:
        virtual ~CrudInterface() = default;

        // Create/read/list/update/delete record(s), use opts to config optional params
        virtual T create(const T& entity, [[maybe_unused]] const json& opts) = 0;
        virtual std::optional<T> read(const std::string& id, [[maybe_unused]] const json& opts) = 0;
        virtual T update(const std::string& id, const T& entity, [[maybe_unused]] const json& opts) = 0;
        virtual bool remove(const std::string& id, [[maybe_unused]] const json& opts) = 0;
        virtual std::vector<T> list([[maybe_unused]] const json& opts) = 0;
    };
}
#endif //CRUD_H
