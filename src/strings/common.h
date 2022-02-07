#pragma once

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <string>
#include <type_traits>

#include "meta/common.h"

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
    // A whitespace sequence is removed completely unless on both sides it has has letters, numbers, or unknown characters (>= 128).
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

    // Splits the string by a specific character.
    // `func` is `R func(std::string_view elem [, bool is_final])`, where `R` is either void or contextually convertible to bool.
    // If it returns a truthy value, the loop stops and true is returned.
    template <typename F>
    bool Split(std::string_view str, char sep, F &&func)
    {
        auto it = str.begin();
        auto elem_start = it;
        while (true)
        {
            bool is_end = it == str.end();
            bool is_sep = !is_end && *it == sep;
            if (is_sep || is_end)
            {
                bool stop;
                if constexpr (std::is_invocable_v<F, std::string_view, bool>)
                    stop = Meta::return_value_or<bool>(false, func, std::string_view(elem_start, it), bool(is_end)); // Note the cast, it protects the original variable.
                else
                    stop = Meta::return_value_or<bool>(false, func, std::string_view(elem_start, it));
                if (stop)
                    return true;
            }

            if (is_end)
                return false;

            it++;

            if (is_sep)
                elem_start = it;
        }
    }
}
