#pragma once

#include <iomanip>
#include <string>
#include <string_view>
#include <sstream>
#include <utility>

#include "utils/unicode.h"

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

    [[nodiscard]] inline bool EndsWith(std::string_view str, std::string_view suffix)
    {
        if (str.size() < suffix.size()) // We don't want `str.size() - suffix.size()` to overflow.
            return 0;
        return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    enum class UseUnicode : bool {};

    struct SymbolPosition
    {
        int line = 0;
        int column = 0;

        std::string ToString() const
        {
            if (line == 0 && column == 0)
                return "";
            else
                return Str(line, ':', column);
        }
    };

    [[nodiscard]] inline SymbolPosition GetSymbolPosition(const char *start, const char *symbol, UseUnicode use_unicode = UseUnicode(1))
    {
        // For extra safety, we swap the pointers if they're ordered incorrectly.
        if (symbol < start)
            std::swap(symbol, start);

        SymbolPosition ret;
        ret.line = 1;
        ret.column = 1;
        std::uint32_t prev_line_end = 0;

        auto lambda = [&](std::uint32_t ch)
        {
            if (ch != '\n' && ch != '\r')
            {
                ret.column++;
                prev_line_end = 0;
                return;
            }

            if (prev_line_end != 0 && ch != prev_line_end)
            {
                prev_line_end = 0;
                return; // Skip a second byte of a line end.
            }

            prev_line_end = ch;
            ret.line++;
            ret.column = 1;
        };

        if (bool(use_unicode))
        {
            for (std::uint32_t ch : Unicode::Iterator(start, symbol))
                lambda(ch);
        }
        else
        {
            for (const char *ptr = start; ptr < symbol; ptr++)
                lambda((unsigned char)*ptr);
        }

        return ret;
    }
}

using Strings::Str;
