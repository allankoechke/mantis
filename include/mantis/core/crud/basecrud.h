//
// Created by allan on 16/05/2025.
//

#ifndef BASECRUD_H
#define BASECRUD_H

#include <optional>
#include <vector>

#include <nlohmann/json.hpp>

#include "crud.h"
using json = nlohmann::json;

#include "../models/models.h"

namespace mantis
{
    class BaseTableCrud : public CrudInterface<json>
    {
    public:
        BaseTableCrud(BaseTable& t) {};
        virtual ~BaseTableCrud() = default;
    };
}


#endif //BASECRUD_H
