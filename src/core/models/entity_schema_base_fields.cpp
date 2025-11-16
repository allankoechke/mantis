//
// Created by codeart on 12/11/2025.
//

#include "../../../include/mantis/core/models/entity_schema.h"
#include "../../../include/mantis/core/models/entity_schema_field.h"

namespace mantis {
    std::vector<EntitySchemaField> EntitySchema::defaultBaseFieldsSchema = {
        EntitySchemaField{
            {
                {"name", "id"},
                {"type", "string"},
                {"required", true},
                {"primary_key", true},
                {"system", true},
                {"unique", false},
                {
                    "constraints", {
                        {"min-value", 6},
                        {"max_value", nullptr},
                        {"validator", "password"},
                        {"default_value", nullptr},
                    }
                },
            }
        },
        EntitySchemaField{
            {
                {"name", "created"},
                {"type", "date"},
                {"required", true},
                {"primary_key", false},
                {"system", true},
                {"unique", false},
                {
                    "constraints", {
                        {"min-value", nullptr},
                        {"max_value", nullptr},
                        {"validator", nullptr},
                        {"default_value", nullptr},
                    }
                },
            }
        },
        EntitySchemaField{
            {
                {"name", "updated"},
                {"type", "date"},
                {"required", true},
                {"primary_key", false},
                {"system", true},
                {"unique", false},
                {
                    "constraints", {
                        {"min-value", nullptr},
                        {"max_value", nullptr},
                        {"validator", nullptr},
                        {"default_value", nullptr},
                    }
                },
            }
        }
    };
    std::vector<EntitySchemaField> EntitySchema::defaultAuthFieldsSchema = {
        EntitySchemaField{
            {
                //User NAME
                {"name", "name"},
                {"type", "string"},
                {"required", true},
                {"primary_key", false},
                {"system", true},
                {"unique", false},
                {
                    "constraints", {
                        {"min-value", 6},
                        {"max_value", nullptr},
                        {"validator", "password"},
                        {"default_value", nullptr},
                    }
                },
            }
        },
        EntitySchemaField{
            {
                // User EMAIL
                {"name", "email"},
                {"type", "string"},
                {"required", true},
                {"primary_key", false},
                {"system", true},
                {"unique", true},
                {
                    "constraints", {
                        {"min-value", nullptr},
                        {"max_value", nullptr},
                        {"validator", "@email"},
                        {"default_value", nullptr},
                    }
                },
            }
        },
        EntitySchemaField{
            {
                {"name", "password"},
                {"type", "string"},
                {"required", true},
                {"primary_key", false},
                {"system", true},
                {"unique", false},
                {
                    "constraints", {
                        {"min-value", 6},
                        {"max_value", nullptr},
                        {"validator", "@password"},
                        {"default_value", nullptr},
                    }
                },
            }
        }
    };

    std::vector<std::string> EntitySchemaField::defaultBaseFields = {"id", "created", "updated"};
    std::vector<std::string> EntitySchemaField::defaultAuthFields = {
        "id", "created", "updated", "name", "email", "password"
    };
    std::vector<std::string> EntitySchemaField::defaultEntityFieldTypes = {
        "xml", "string", "double", "date", "int8", "uint8", "int16", "uint16", "int32", "uint32", "int64", "uint64",
        "blob", "json", "bool", "file", "files"
    };
} // mantis
