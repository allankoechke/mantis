/**
 * @file files_mgr.cpp
 * @brief Implementation for @see files_mgr.h
 */

#include "../../include/mantisbase/core/files_mgr.h"
#include "../../include/mantisbase/core/logs_mgr.h"
#include "../../include/mantisbase/mantisbase.h"

#include <fstream>
#include <filesystem>

namespace mantis
{
    namespace fs = std::filesystem;

    void FilesMgr::createDir(const std::string& table) const
    {
        logger::trace("Creating directory: {}", dirPath(table));

        if (table.empty()) return;

        if (const auto path = dirPath(table); !fs::exists(path))
            fs::create_directories(path);
    }

    void FilesMgr::renameDir(const std::string& old_name, const std::string& new_name) const
    {
        logger::trace("Renaming folder name from '{}' to '{}'", old_name, new_name);

        // Rename folder if it exists, else, create it
        if (const auto old_path = dirPath(old_name); fs::exists(old_path))
            fs::rename(old_path, dirPath(new_name));

        else
            createDir(new_name);
    }

    void FilesMgr::deleteDir(const std::string& table) const
    {
        logger::trace("Removing {}", dirPath(table));
        fs::remove_all(dirPath(table));
    }

    std::optional<std::string> FilesMgr::getFilePath(const std::string& table, const std::string& filename) const
    {
        // Check if file exists, if so, return the path, else, return std::nullopt
        if (auto path = filePath(table, filename); fs::exists(path))
        {
            return path;
        }

        return std::nullopt;
    }

    // bool FileUnit::saveFile(const std::string& table, const std::string& filename,
    //                         const std::vector<uint8_t>& content) const
    // {
    //     std::ofstream file(filePath(table, filename), std::ios::binary);
    //     if (!file) return false;
    //     file.write(reinterpret_cast<const char*>(content.data()), content.size());
    //     return true;
    // }

    std::string FilesMgr::dirPath(const std::string& table, bool create_if_missing) const
    {
        auto path = (fs::path(MantisBase::instance().dataDir()) / "files" / table).string();
        if (!fs::exists(path) && create_if_missing)
        {
            fs::create_directories(path);
        }
        return path;
    }

    std::string FilesMgr::filePath(const std::string& table, const std::string& filename) const
    {
        return (fs::path(MantisBase::instance().dataDir()) / "files" / table / filename).string();
    }

    bool FilesMgr::removeFile(const std::string& table, const std::string& filename) const
    {
        try
        {
            if (table.empty() || filename.empty())
            {
                logger::warn("Table name and filename are required!");
                return false;
            }

            const auto path = filePath(table, filename);
            logger::trace("Removing file at `{}`", path);

            // Remove the file, only if it exists
            if (fs::exists(path))
            {
                fs::remove(path);
                return true;
            }

            logger::warn("Could not remove file at `{}`, seems to be missing!", path);
        }
        catch (const std::exception& e)
        {
            logger::critical("Error removing file: {}", e.what());
        }

        return false;
    }
} // mantis
