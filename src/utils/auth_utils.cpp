#include "../../include/mantis/utils/utils.h"
#include "../../include/mantis/app/app.h"

#include <bcrypt-cpp/bcrypt.h>

namespace mantis
{
    std::string hashPassword(const std::string& password)
    {
        return bcrypt::generateHash(password);
    }

    bool verifyPassword(const std::string& password, const std::string& stored_hash)
    {
        return bcrypt::validatePassword(password, stored_hash);
    }
}
