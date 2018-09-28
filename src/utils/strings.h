#pragma once

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

    [[nodiscard]] inline bool StartsWith(std::string_view str, std::string_view prefix)
    {
        // We don't need to check `str.size() < prefix.size()` since substring length is allowed to be larger than necessary.
        return str.compare(0, prefix.size(), prefix) == 0;
    }

    [[nodiscard]] inline bool EndsWith(std::string_view str, std::string_view prefix)
    {
        if (str.size() < prefix.size()) // We don't want `str.size() - prefix.size()` to overflow.
            return 0;
        return str.compare(str.size() - prefix.size(), prefix.size(), prefix) == 0;
    }
}

using Strings::Str;
