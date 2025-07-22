//
// Created by allan on 17/07/2025.
//

#include "../../include/mantis/core/fileunit.h"
#include "../../include/mantis/core/logging.h"
#include "../../include/mantis/app/app.h"

#include <fstream>
#include <filesystem>

#include "../../include/mantis/core/logging.h"

namespace mantis
{
    namespace fs = std::filesystem;

    void FileUnit::createDir(const std::string& table)
    {
        if (table.empty()) return;

        if (const auto path = MantisApp::instance().dataDir() + "/files/" + table;
            !fs::exists(path))
            fs::create_directories(path);
    }

    void FileUnit::renameDir(const std::string& old_name, const std::string& new_name)
    {
        // Rename folder if it exists, else, create it
        if (const auto old_path = MantisApp::instance().dataDir() + "/" + old_name;
            fs::exists(old_path))
            fs::rename(old_path,
                       MantisApp::instance().dataDir() + "/files/" + new_name);
        else
            createDir(new_name);
    }

    void FileUnit::deleteDir(const std::string& table)
    {
        fs::remove_all(MantisApp::instance().dataDir() + "/files/" + table);
    }

    std::string FileUnit::filePath(const std::string& table, const std::string& filename) const
    {
        return (fs::path(MantisApp::instance().dataDir()) / table / filename).string();
    }

    std::optional<std::string> FileUnit::getFilePath(const std::string& table, const std::string& filename) const
    {
        // Check if file exists, if so, return the path, else, return std::nullopt
        if (auto path = filePath(table, filename); fs::exists(path))
        {
            return path;
        }

        return std::nullopt;
    }

    bool FileUnit::saveFile(const std::string& table, const std::string& filename,
                            const std::vector<uint8_t>& content) const
    {
        std::ofstream file(filePath(table, filename), std::ios::binary);
        if (!file) return false;
        file.write(reinterpret_cast<const char*>(content.data()), content.size());
        return true;
    }

    bool FileUnit::removeFile(const std::string& table, const std::string& filename) const
    {
        try
        {
            const auto path = filePath(table, filename);

            // Remove the file, only if it exists
            if (fs::exists(path))
            {
                fs::remove(path);
                return true;
            }

            Log::warn("Could not remove file at `{}`, seems to be missing!", path);
        }
        catch (const std::exception& e)
        {
            Log::critical("Error removing file: {}", e.what());
        }

        return false;
    }
} // mantis
