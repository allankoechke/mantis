//
// Created by allan on 22/06/2025.
//
#include "../../include/mantis/utils/utils.h"
#include <sstream>
#include <iomanip>
#include <libbcrypt.h>

namespace mantis
{
    static constexpr char BCRYPT_BASE64[] =
        "./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    std::string bcryptBase64Encode(const unsigned char* data, size_t len)
    {
        std::string out;
        int c1, c2, c3;

        for (size_t i = 0; i < len;)
        {
            c1 = data[i++];
            c2 = (i < len) ? data[i++] : 0;
            c3 = (i < len) ? data[i++] : 0;

            out += BCRYPT_BASE64[(c1 >> 2) & 0x3f];
            out += BCRYPT_BASE64[((c1 & 0x3) << 4) | ((c2 & 0xf0) >> 4)];
            out += BCRYPT_BASE64[((c2 & 0xf) << 2) | ((c3 & 0xc0) >> 6)];
            out += BCRYPT_BASE64[c3 & 0x3f];
        }

        return out.substr(0, 22); // bcrypt wants 22-character base64 salt
    }


    std::string generateSalt(const int cost)
    {
        std::random_device rd;
        unsigned char raw[16];
        for (unsigned char& i : raw)
            i = static_cast<unsigned char>(rd());

        std::ostringstream oss;
        oss << "$2a$";
        oss << std::setw(2) << std::setfill('0') << cost << "$";
        oss << bcryptBase64Encode(raw, sizeof(raw));
        return oss.str();
    }

    json hashPassword(const std::string& password)
    {
        std::string salt;

        // Response object
        json response;
        response["hash"] = "";
        response["salt"] = "";
        response["error"] = "";

        try
        {
            salt = generateSalt();
        }
        catch (const std::exception& e)
        {
            response["error"] = e.what();
            return response;
        }

        // Hash the password with the generated salt
        char hash[BCRYPT_HASHSIZE];
        if (bcrypt_hashpw(password.c_str(), salt.c_str(), hash) != 0)
        {
            response["error"] = "Password hashing failed!";
        }

        else
        {
            response["hash"] = std::string(hash);
            response["salt"] = std::string(salt);
        }

        return response;
    }

    json verifyPassword(const std::string& password, const std::string& stored_hash)
    {
        json response{{"verified", false}, {"reason", ""}};

        if (const int ret = bcrypt_checkpw(password.c_str(), stored_hash.c_str()); ret == -1)
        {
            response["error"] = "Error occurred while verifying password!";
        }
        else if (ret == 0)
        {
            response["verified"] = true;
        }
        else
        {
            response["error"] = "Password does not match";
        }

        return response;
    }
}
