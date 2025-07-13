/**
 * @file utils.h
 *
 * @brief Collection of utility functions that are re-used across different files.
 *
 * Created by allan on 11/05/2025.
 */

#ifndef MANTIS_UTILS_H
#define MANTIS_UTILS_H

#include <string>
#include <filesystem>
#include <random>

#include "../core/logging.h"

namespace mantis
{
    namespace fs = std::filesystem; ///< Use shorthand `fs` to refer to the `std::filesystem`

    // ----------------------------------------------------------------- //
    // PATH UTILS
    // ----------------------------------------------------------------- //

    /**
     * @brief Joins absolute path and a relative path, relative to the first path.
     * @param path1 The first absolute path or relative path
     * @param path2 The relative path, subject to the first
     * @return An absolute path if successfully joined, else an empty path.
     */
    fs::path joinPaths(const std::string& path1, const std::string& path2);

    /**
     * Resolves given path as a string to an absolute path.
     *
     * Given a relative path, relative to the `cwd`, we can resolve that path to the actual
     * absolute path in our filesystem. This is needed for creating directories and files, especially
     * for database and file-serving.
     *
     * @param input_path The path to resolve
     * @return Returns an absolute filesystem path.
     */
    fs::path resolvePath(const std::string& input_path);

    /**
     * @brief Create directory, given a path
     *
     * Creates the target directory iteratively, including any missing parent directories.
     * his ensures, any parent directory is set up before attempting to create a child directory.
     *
     * @param path The directory path to create like `/foo/bar`.
     * @return True if creation was successful. If the directory exists, it returns false.
     */
    bool createDirs(const fs::path& path);

    /**
     * @brief Returns a created/existing directory from a path.
     *
     * Given a file path, it first gets the file parent directory and ensures the directory
     * together with any missing parent directories are created first using the `createDirs(...)`.
     *
     * @see createDirs() for creating directories.
     *
     * @param path The file path
     * @return Returns the directory path if successful, else an empty string.
     */
    std::string dirFromPath(const std::string& path);

    // ----------------------------------------------------------------- //
    // STRING UTILS
    // ----------------------------------------------------------------- //
    /**
     * @brief Converts a string to its lowercase variant.
     *
     * It converts the string in place.
     *
     * @param str The string to convert.
     * @see toUpperCase() To convert string to uppercase.
     */
    void toLowerCase(std::string& str);

    /**
     * @brief Converts a string to its uppercase variant.
     *
     * It converts the string in place.
     *
     * @param str The string to convert.
     * @see toLowerCase() To convert string to lowercase.
     */
    void toUpperCase(std::string& str);

    /**
     * @brief Trims leading and trailing whitespaces from a string.
     *
     * @param s The string to trim.
     * @return String with all leading and trailing whitespaces removed.
     */
    std::string trim(const std::string& s);

    /**
     * @brief Attempt to parse a JSON string.
     * @param json_str JSON string to parse
     * @return A JSON Object if successful, else a `std::nullopt`
     *
     * @code
     * auto user = tryParseJsonStr("{\"name\": \"John Doe\"}");
     * if(user.has_value()) {
     *      // Do something ...
     * }
     * @endcode
     */
    std::optional<json> tryParseJsonStr(const std::string& json_str);

    /**
     * @brief Convert given string value to boolean type.
     *
     * By default, we check for true equivalents, anything else will be
     * considered as a false value.
     *
     * @param value String value to convert to bool
     * @return true or false value
     */
    bool strToBool(const std::string& value);

    /**
     * @brief Generate a time base UUID
     *
     * The first part is made up of milliseconds since epoch while the last 4 digits a random component.
     * This makes it lexicographically sortable by time
     *
     * Sample Output: `17171692041233276`
     *
     * @return A time based UUID
     *
     * @see generateReadableTimeId() For a readable time-based UUID.
     * @see generateShortId() For a short UUID.
     */
    std::string generateTimeBasedId();

    /**
     * @brief Generates a readable time-based UUID.
     *
     * The first segment is the current time in ISO-formatted time + milliseconds + short random suffix.
     * It is human-readable and sortable, just like
     *
     * Sample Output: `20250531T221944517N3J`
     *
     * @return A readable-time based UUID
     *
     * @see generateTimeBasedId() For a time-based UUID.
     * @see generateShortId() For a short UUID.
    */
    std::string generateReadableTimeId();

    /**
     * @brief Generates a short UUID
     *
     * Similar to what platforms like YouTube use for videos, but in our case, making use of only
     * alphanumeric characters.
     *
     * Sample Output: `Fz8xYc6a7LQw`
     *
     * @return A short alphanumeric UUID
     *
     * @see generateTimeBasedId() For a time-based UUID.
     * @see generateReadableTimeId() For a readable time-based UUID.
     */
    std::string generateShortId(size_t length = 12);

    /**
     * @brief Split given string based on given delimiter
     *
     * @param input Input string to split.
     * @param delimiter The string delimiter to use to split the `input` string.
     * @return A vector of strings.
     *
     * @code
     * auto parts = splitString("Hello, John!", ",");
     * std::cout << parts.size() << std::endl;
     * // > Should be a vector of two strings `Hello` and ` World`
     * @endcode
     */
    std::vector<std::string> splitString(const std::string& input, const std::string& delimiter);

    /**
     * @brief Retrieves a value from an environment variable or a default value if the env variable was not set.
     * @param key Environment variable key.
     * @param defaultValue A default value if the key is not set.
     * @return The env value if found, else the default value passed in.
     */
    std::string getEnvOrDefault(const std::string& key, const std::string& defaultValue);


    // ----------------------------------------------------------------- //
    // AUTH UTILS
    // ----------------------------------------------------------------- //
    /**
     * @brief Encode a Salt string to bcrypt base64 format.
     * @param data `char*` string data to encode.
     * @param len Length of the string data.
     * @return A base64 encoded salt value.
     */
    std::string bcryptBase64Encode(const unsigned char* data, size_t len);

    /**
     * @brief Generates a salt to be used in hashing user passwords.
     * @param cost Cost parameter.
     * @return A base64 encoded salt value.
     *
     * @see https://app.studyraid.com/en/read/12358/398958/understanding-the-cost-factor-parameter
     */
    std::string generateSalt(int cost = 12);

    /**
     * @brief Digests user password + a generated salt to yield a hashed password.
     * @param password Password input to hash.
     * @return A JSON object having `hash`, `salt` and `error` values.
     */
    json hashPassword(const std::string& password);

    /**
     * @brief Verifies user password if it matches the given hashed password.
     *
     * Given a hashed password from the database (`stored_hash`), the method extracts the salt value,
     * hashes the `password` value with the salt then compares the two hashes if they match.
     *
     * @param password User password input.
     * @param stored_hash Database stored hashed user password.
     * @return JSON object indicating whether the verification was successful and an error value if any.
     */
    json verifyPassword(const std::string& password, const std::string& stored_hash);

}

#endif // MANTIS_UTILS_H
