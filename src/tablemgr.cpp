//
// Created by allan on 13/05/2025.
//

#include "../include/mantis/api/tablemgr.h"

#include <mantis/mantis.h>

Mantis::TableMgr::TableMgr(const MantisApp& app)
: m_app(make_shared<MantisApp>(app))
{}

void Mantis::TableMgr::FetchRecord(const Request& req, Response& res, Context& ctx)
{

}

void Mantis::TableMgr::FetchRecords(const Request& req, Response& res, Context& ctx)
{

}

void Mantis::TableMgr::CreateRecord(const Request& req, Response& res, Context& ctx)
{

}

void Mantis::TableMgr::UpdateRecord(const Request& req, Response& res, Context& ctx)
{

}

void Mantis::TableMgr::DeleteRecord(const Request& req, Response& res, Context& ctx)
{

}

bool Mantis::TableMgr::AuthWithPassword(const std::string& email, std::string& password)
{
    return true;
}
