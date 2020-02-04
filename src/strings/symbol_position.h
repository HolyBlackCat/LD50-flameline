#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include "utils/unicode.h"

namespace Strings
{
    enum class UseUnicode : bool {};

    struct SymbolPosition
    {
        int line = 1;
        int column = 1;

        std::string ToString() const
        {
            return Str(line, ':', column);
        }

        class State
        {
            friend SymbolPosition;
            Unicode::Char prev_line_end = 0;
          public:
            State() {}
        };

        // It doesn't matter what exactly `ch` is, as long as it's not equal to '\r' or '\n';
        void AddSymbol(Unicode::Char ch, State &state)
        {
            if (ch != '\n' && ch != '\r')
            {
                column++;
                state.prev_line_end = 0;
                return;
            }

            if (state.prev_line_end != 0 && ch != state.prev_line_end)
            {
                state.prev_line_end = 0;
                return; // Skip the second byte of a line end.
            }

            state.prev_line_end = ch;
            line++;
            column = 1;
        }

        void AddSymbol(char ch, State &state)
        {
            // Yes, the cast will give huge negative values for negative characters, but it doesn't matter.
            AddSymbol(Unicode::Char(ch), state);
        }
    };

    [[nodiscard]] inline SymbolPosition GetSymbolPosition(const char *start, const char *symbol, UseUnicode use_unicode = UseUnicode(1))
    {
        // For extra safety, we swap the pointers if they're ordered incorrectly.
        if (symbol < start)
            std::swap(symbol, start);

        SymbolPosition ret;
        SymbolPosition::State state;

        if (bool(use_unicode))
        {
            for (Unicode::Char ch : Unicode::Iterator(start, symbol))
                ret.AddSymbol(ch, state);
        }
        else
        {
            for (const char *ptr = start; ptr < symbol; ptr++)
                ret.AddSymbol(*ptr, state);
        }

        return ret;
    }
}
