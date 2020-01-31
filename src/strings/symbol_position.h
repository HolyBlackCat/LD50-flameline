#pragma once

#include <string>
#include <utility>
#include <cstdint>

#include "utils/unicode.h"

namespace Strings
{
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
