#include "../../include/mantis/utils/utils.h"
#include <private/soci-mktime.h>

namespace mantis
{
    std::string tmToStr(const std::tm& t)
    {
        char buffer[80];
        const int length = soci::details::format_std_tm(t, buffer, sizeof(buffer));
        std::string iso_string(buffer, length);
        return iso_string;
    }

    std::tm strToTM(const std::string& value)
    {
        auto dt = value;
        std::tm t;

        auto pos = dt.find('T');
        if (pos != std::string::npos) dt[pos] = ' ';
        soci::details::parse_std_tm(dt.c_str(), t);

        return t;
    }

    std::string dbDateToString(const std::string& dbType, const soci::row& row, const int index)
    {
        if (dbType == "sqlite3")
        {
            // Treat date as a string by default
            return row.get<std::string>(index);
        }

        // if (row.get_properties(index).get_db_type() == soci::db_date)
        const auto t = row.get<std::tm>(index);
        return tmToStr(t);
    }
}
