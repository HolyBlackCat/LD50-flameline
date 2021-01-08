#pragma once

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <string>

namespace Strings
{
    // Passes all parameters to a temporary string stream, and returns the resulting string.
    template <typename C = char, typename ...P>
    [[nodiscard]] std::basic_string<C> StreamConcat(const P &... params)
    {
        std::basic_ostringstream<C> stream;
        (stream << ... << params);
        return stream.str();
    }

    // Trims the string by removing any leading and trailing whitespace.
    [[nodiscard]] inline std::string_view Trim(std::string_view str)
    {
        static constexpr char chars_to_remove[] = " \n\r\t\v\f";
        str.remove_prefix(std::min(str.size(), str.find_first_not_of(chars_to_remove)));
        if (str.size())
            str.remove_suffix(str.size() - str.find_last_not_of(chars_to_remove) - 1);
        return str;
    }

    // Trims the string, and either replaces any sequences of whitespace characters with single spaces, or removes them completely.
    // A whitespace sequences is removed completely unless on both sides it has has letters, numbers, or unknown characters (>= 128).
    [[nodiscard]] inline std::string Condense(std::string_view str)
    {
        auto IsLetter = [&](unsigned char ch)
        {
            return std::isalnum(ch) || ch >= 128;
        };

        str = Trim(str);
        std::string ret;
        char prev = 0;
        bool got_space = false;
        for (unsigned char ch : str)
        {
            if (!std::isspace(ch))
            {
                if (got_space)
                {
                    got_space = false;
                    if (IsLetter(ch))
                        ret += ' ';
                }

                ret += ch;
            }
            else if (IsLetter(prev))
                got_space = true;

            prev = ch;
        }

        return ret;
    }

    [[nodiscard]] inline std::string Replace(std::string_view source, std::string_view a, std::string_view b)
    {
        std::string ret;

        std::size_t cur_pos;
        std::size_t last_pos = 0;

        while ((cur_pos = source.find(a, last_pos)) != std::string::npos)
        {
            ret.append(source, last_pos, cur_pos - last_pos);
            ret += b;
            last_pos = cur_pos + a.length();
        }

        ret += source.substr(last_pos);
        return ret;
    }
}
