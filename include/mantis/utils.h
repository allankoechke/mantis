//
// Created by allan on 11/05/2025.
//

#ifndef MANTIS_UTILS_H
#define MANTIS_UTILS_H

#include <string>
#include <algorithm>

namespace Mantis
{
    inline void ToLowerCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
            [](const unsigned char c){ return std::tolower(c); });
    }

    inline void ToUpperCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
            [](const unsigned char c){ return std::toupper(c); });
    }

    inline fs::path JoinPaths(const std::string& path1, const std::string& path2)
    {
        fs::path result = fs::path(path1) / fs::path(path2);
        return result;
    }
}

#endif // MANTIS_UTILS_H
