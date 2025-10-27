#include "../../../include/mantis/core/tables/tables.h"
#include "../../../include/mantis/core/database.h"
#include "../../../include/mantis/utils/utils.h"

#define __file__ "core/tables/tables.cpp"

namespace mantis
{
    TableUnit::TableUnit(
        std::string tableName,
        std::string tableId,
        std::string tableType)
        : m_tableName(std::move(tableName)),
          m_tableId(std::move(tableId)),
          m_tableType(std::move(tableType)),
          m_listRule(Rule{}),
          m_getRule(Rule{}), m_addRule(Rule{}),
          m_updateRule(Rule{}), m_deleteRule(Rule{})
    {
    }

    TableUnit::TableUnit(
        const json& schema)
        : m_listRule(Rule{}),
          m_getRule(Rule{}),
          m_addRule(Rule{}),
          m_updateRule(Rule{}),
          m_deleteRule(Rule{}) { fromJson(schema); }

    void TableUnit::fromJson(const json& j)
    {
        // Ensure table name is passed in by the caller ...
        if (j.value("name", "").empty())
            throw std::invalid_argument("empty table name");

        // Set table name from JSON
        m_tableName = j.value("name", "");
        m_tableId = generateTableId(m_tableName);

        m_fields.clear();
        m_fields = j.value("fields", json::array());

        m_listRule = j.value("listRule", "");
        m_getRule = j.value("getRule", "");
        m_addRule = j.value("addRule", "");
        m_updateRule = j.value("updateRule", "");
        m_deleteRule = j.value("deleteRule", "");

        m_isSystem = j.value("system", false);
        m_tableType = j.value("type", "base");
    }

    void TableUnit::setRouteDisplayName(const std::string& routeName)
    {
        if (routeName.empty())
            return;

        m_routeName = routeName;
    }

    std::string TableUnit::tableName()
    {
        return m_tableName;
    }

    void TableUnit::setTableName(const std::string& name)
    {
        m_tableName = name;
    }

    std::string TableUnit::tableId()
    {
        return m_tableId;
    }

    void TableUnit::setTableId(const std::string& id)
    {
        m_tableId = id;
    }

    std::string TableUnit::tableType()
    {
        return m_tableType;
    }

    std::vector<json> TableUnit::fields() const
    {
        return m_fields;
    }

    void TableUnit::setFields(const std::vector<json>& fields)
    {
        m_fields.clear();
        for (const auto& field : fields) m_fields.push_back(field);
    }

    bool TableUnit::isSystem() const
    {
        return m_isSystem;
    }

    void TableUnit::setIsSystemTable(const bool isSystemTable)
    {
        m_isSystem = isSystemTable;
    }

    Rule TableUnit::listRule()
    {
        return m_listRule;
    }

    void TableUnit::setListRule(const Rule& rule)
    {
        m_listRule = rule;
    }

    Rule TableUnit::getRule()
    {
        return m_getRule;
    }

    void TableUnit::setGetRule(const Rule& rule)
    {
        m_getRule = rule;
    }

    Rule TableUnit::addRule()
    {
        return m_addRule;
    }

    void TableUnit::setAddRule(const Rule& rule)
    {
        m_addRule = rule;
    }

    Rule TableUnit::updateRule()
    {
        return m_updateRule;
    }

    void TableUnit::setUpdateRule(const Rule& rule)
    {
        m_updateRule = rule;
    }

    Rule TableUnit::deleteRule()
    {
        return m_deleteRule;
    }

    void TableUnit::setDeleteRule(const Rule& rule)
    {
        m_deleteRule = rule;
    }
}
