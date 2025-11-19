//
// Created by allan on 22/06/2025.
//

#ifndef CONFIG_HPP_IN_H
#define CONFIG_HPP_IN_H

#define MANTIS_VERSION_MAJOR 0
#define MANTIS_VERSION_MINOR 3
#define MANTIS_VERSION_PATCH 0
#define MANTIS_VERSION_SUFFIX "-dev+g7891af6"
#define MANTIS_GIT_COMMIT "7891af6"
#define MANTIS_VERSION "0.3.0-dev+g7891af6"

namespace mantis {
    inline const char* getVersionString() {
        return MANTIS_VERSION;
    }

    inline const char* getGitCommit() {
        return MANTIS_GIT_COMMIT;
    }

    inline constexpr std::tuple<int, int, int> getVersionTuple() {
        return { MANTIS_VERSION_MAJOR, MANTIS_VERSION_MINOR, MANTIS_VERSION_PATCH };
    }

    inline constexpr bool isVersionAtLeast(int major, int minor, int patch) {
        return std::tuple(MANTIS_VERSION_MAJOR, MANTIS_VERSION_MINOR, MANTIS_VERSION_PATCH)
             >= std::tuple(major, minor, patch);
    }
} // namespace mantis



#endif //CONFIG_HPP_IN_H
