//
// Created by allan on 17/07/2025.
//

#ifndef FILEUNIT_H
#define FILEUNIT_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace mantis
{
    class FileUnit
    {
    public:
        FileUnit() = default;

        void createDir(const std::string& table) const;
        void renameDir(const std::string& old_name, const std::string& new_name) const;
        void deleteDir(const std::string& table) const;

        bool saveFile(const std::string& table, const std::string& filename,
                      const std::vector<std::uint8_t>& content) const;

        std::string dirPath(const std::string& table, bool create_if_missing = false) const;
        std::string filePath(const std::string& table, const std::string& filename) const;
        std::optional<std::string> getFilePath(const std::string& table, const std::string& filename) const;
        bool removeFile(const std::string& table, const std::string& filename) const;
    };
} // mantis

#endif //FILEUNIT_H
