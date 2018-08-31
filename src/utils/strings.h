#ifndef UTILS_STRINGS_H_INCLUDED
#define UTILS_STRINGS_H_INCLUDED

#include <iomanip>
#include <string>
#include <string_view>
#include <sstream>

namespace Strings
{
    template <typename ...P> [[nodiscard]] std::string Str(const P &... params)
    {
        std::ostringstream stream;
        (stream << ... << params);
        return stream.str();
    }

    [[nodiscard]] inline std::string_view Trim(std::string_view str)
    {
        static constexpr char chars_to_remove[] = " \n\r\t";
        str.remove_prefix(std::min(str.size(), str.find_first_not_of(chars_to_remove)));
        if (str.size())
            str.remove_suffix(str.size() - str.find_last_not_of(chars_to_remove) - 1);
        return str;
    }
}

using Strings::Str;

#endif
