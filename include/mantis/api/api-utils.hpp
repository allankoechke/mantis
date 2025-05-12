//
// Created by allan on 12/05/2025.
//

#ifndef API_UTILS_HPP
#define API_UTILS_HPP

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class IApiUnitBase {
public:
    virtual ~IApiUnitBase() {}
    virtual void GetRecord(const std::string& id, const json& params) = 0;
    virtual void GetRecords(const json& params) = 0;
    virtual void AddRecord(const json& data) = 0;
    virtual void UpdateRecord(const std::string& id, const json& data) = 0;
    virtual void RemoveRecord(const std::string& id) = 0;

protected:
    std::string m_tableId;
    std::string m_tableName;
};

class IApiUnitView {
public:
    virtual ~IApiUnitView() {}
    virtual void GetRecord(const std::string& id, const json& params) = 0;
    virtual void GetRecords(const json& params) = 0;

protected:
    std::string m_tableId;
    std::string m_tableName;
};

class IApiUnitAuth
{
public:
    virtual ~IApiUnitAuth() {}
    virtual void GetRecord(const std::string& id, const json& params) = 0;
    virtual void GetRecords(const json& params) = 0;
    virtual void AddRecord(const json& data) = 0;
    virtual void UpdateRecord(const std::string& id, const json& data) = 0;
    virtual void RemoveRecord(const std::string& id) = 0;

    virtual void AuthWithEmailAndPassword(const std::string& email, const std::string& password) = 0;

protected:
    std::string m_tableId;
    std::string m_tableName;
};

class AdminMgr : public IApiUnitAuth
{
public:
    void GetRecord(const std::string& id, const json& params)
    {

    }
    void GetRecords(const json& params)
    {

    }
    void AddRecord(const json& data)
    {

    }
    void UpdateRecord(const std::string& id, const json& data)
    {

    }
    void RemoveRecord(const std::string& id)
    {

    }

    void AuthWithEmailAndPassword(const std::string& email, const std::string& password)
    {

    }
};

#endif //API_UTILS_HPP
