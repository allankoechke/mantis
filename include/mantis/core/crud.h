//
// Created by allan on 16/05/2025.
//

#ifndef CRUD_H
#define CRUD_H

#include <optional>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace mantis {

 template <typename T>
 class CrudInterface {
 public:
     virtual ~CrudInterface() = default;

     // Create/read/list/update/delete record(s), use opts to config optional params
     virtual T& create(const T& entity, const json& opts=json{}) = 0;
     virtual std::optional<T> read(int id, const json& opts=json{}) = 0;
     virtual T& update(int id, const T& entity, const json& opts=json{}) = 0;
     virtual bool remove(int id, const json& opts=json{}) = 0;
     virtual std::vector<T> list(const json& opts=json{}) = 0;
 };

class BaseCrudInterface {

};
}



#endif //CRUD_H
