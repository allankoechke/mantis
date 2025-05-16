//
// Created by allan on 11/05/2025.
//

#ifndef MANTIS_UTILS_H
#define MANTIS_UTILS_H

#include <string>
#include <algorithm>

namespace mantis
{
    inline void toLowerCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
            [](const unsigned char c){ return std::tolower(c); });
    }

    inline void toUpperCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
            [](const unsigned char c){ return std::toupper(c); });
    }

    inline fs::path joinPaths(const std::string& path1, const std::string& path2)
    {
        fs::path result = fs::path(path1) / fs::path(path2);
        return result;
    }

    inline fs::path resolvePath(const std::string& input_path)
    {
        fs::path path(input_path);

        if (!path.is_absolute())
        {
            // Resolve relative to app binary
            path = fs::absolute(path);
        }

        return path;
    }

    inline bool createDirs(const fs::path& path)
    {
        try
        {
            if (!fs::exists(path))
            {
                fs::create_directories(path); // creates all missing parent directories too
            }

            return true;
        }
        catch (const fs::filesystem_error& e)
        {
            Log::critical("Filesystem error while creating directory '{}', reason: {}",
                path.string(), e.what());
            return false;
        }

    }

    std::string dirFromPath(const std::string& path)
    {
        if (const auto dir = resolvePath(path); createDirs(dir))
            return dir.string();

        return "";
    }

}

#endif // MANTIS_UTILS_H
