#pragma once

#include <array>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>

#include "program/errors.h"
#include "macros/check.h"
#include "utils/unicode.h"

namespace Strings
{
    enum class EscapeFlags
    {
        no_flags = 0,
        escape_extended_chars = 1 << 0, // Escape 128-255 characters.
        multiline             = 1 << 1, // Don't escape `\n`.
        strip_cr              = 1 << 2, // Remove `\r`.
        multiline_without_cr  = multiline | strip_cr,
        escape_single_quotes  = 1 << 3, // Escape '.
        escape_double_quotes  = 1 << 4, // Escape ".
    };
    [[nodiscard]] inline EscapeFlags operator&(EscapeFlags a, EscapeFlags b) {return EscapeFlags(int(a) & int(b));}
    [[nodiscard]] inline EscapeFlags operator|(EscapeFlags a, EscapeFlags b) {return EscapeFlags(int(a) | int(b));}

    // Escapes a string.
    // By default, all control characters are escaped, including `\n` and `\r`, and extended (>= 128) characters are not escaped.
    // If a character can't be escaped with a single symbol (\?), then \xNN is always used.
    template <typename Iter, CHECK_EXPR(*std::declval<Iter &>()++ = char())>
    void Escape(std::string_view str, Iter output_iter, EscapeFlags flags = EscapeFlags::no_flags)
    {
        auto OutputString = [&](const char *ptr)
        {
            while (*ptr)
                *output_iter++ = *ptr++;
        };

        for (unsigned char ch : str)
        {
            // Skip `\r` if we have the `strip_cr` flag.
            if (bool(flags & EscapeFlags::strip_cr) && ch == '\r')
                continue;

            bool should_escape =
                // Escape 0..31, except for `\n` if we have the `multiline` flag.
                (ch < ' ' && (!bool(flags & EscapeFlags::multiline) || ch != '\n')) ||
                // Escape `DEL`.
                ch == 0x7f ||
                // Escape 128..255 if we have the `escape_extended_chars` flag.
                (bool(flags & EscapeFlags::escape_extended_chars) && ch >= 128) ||
                // Escape single quotes if the corresponding flag is set.
                (bool(flags & EscapeFlags::escape_single_quotes) && ch == '\'') ||
                // Escape double quotes if the corresponding flag is set.
                (bool(flags & EscapeFlags::escape_double_quotes) && ch == '\"');

            if (!should_escape)
            {
                *output_iter++ = ch;
                continue;
            }

            switch (ch)
            {
                case '\0': OutputString(R"(\0)"); break;
                case '\'': OutputString(R"(\')"); break;
                case '\"': OutputString(R"(\")"); break;
                case '\\': OutputString(R"(\\)"); break;
                case '\a': OutputString(R"(\a)"); break;
                case '\b': OutputString(R"(\b)"); break;
                case '\f': OutputString(R"(\f)"); break;
                case '\n': OutputString(R"(\n)"); break;
                case '\r': OutputString(R"(\r)"); break;
                case '\t': OutputString(R"(\t)"); break;
                case '\v': OutputString(R"(\v)"); break;

              default:
                {
                    char buffer[5]; // 5 bytes for: \ x N N \0
                    std::snprintf(buffer, sizeof buffer, "\\x%02X", ch);
                    OutputString(buffer);
                }
                break;
            }
        }
    }
    [[nodiscard]] inline std::string Escape(std::string_view str, EscapeFlags flags = EscapeFlags::no_flags)
    {
        std::string ret;
        Escape(str, std::back_inserter(ret), flags);
        return ret;
    }

    [[nodiscard]] inline std::string Escape(char ch)
    {
        return Escape({&ch, 1});
    }


    enum class UnescapeFlags
    {
        no_flags = 0,
        strip_cr_bytes = 1 << 2, // Remove `\r` bytes from the input. Escape sequences `\r` are not affected, only the raw bytes are.
    };
    [[nodiscard]] inline UnescapeFlags operator&(UnescapeFlags a, UnescapeFlags b) {return UnescapeFlags(int(a) & int(b));}
    [[nodiscard]] inline UnescapeFlags operator|(UnescapeFlags a, UnescapeFlags b) {return UnescapeFlags(int(a) | int(b));}

    // Unescapes a string. Throws on failure.
    // Supports following escape sequences: \', \", \\, \a, \b, \f, \n, \r, \t, \v.
    // Doesn't support \?, because it's stupid.
    // Additionally supports octal \[0-7]{1,3}, hex \x[a-zA-Z0-9]{1,2}, and unicode \u[a-zA-Z0-9]{4}, \U[a-zA-Z0-9]{8} escapes.
    template <typename Iter, CHECK_EXPR(*std::declval<Iter &>()++ = char())>
    void Unescape(std::string_view str, Iter output_iter, UnescapeFlags flags = UnescapeFlags::no_flags)
    {
        auto cur = str.begin();
        const auto end = str.end();

        std::array<char, 9> buffer; // The max amount of digits we might need to read is 8 (for \U escape), and we need room for a null-terminator.

        // `pred` is `bool pred(char)`.
        // Fills `buffer` with at most `max_count` symbols, stops if `pred` returns `false` for a symbol.
        // Adds a null terminator.
        // Returns the amount of extracted symbols, not including the null-terminator.
        auto ReadSeveralSymbols = [&cur, &end, &buffer](std::size_t max_count, auto pred) -> int
        {
            assert(max_count <= buffer.size() - 1);
            std::size_t buf_pos = 0;
            while (cur < end && buf_pos < max_count && pred(*cur))
                buffer[buf_pos++] = *cur++;
            buffer[buf_pos] = '\0';
            return buf_pos;
        };

        auto IsOctalDigit = [](char ch){return ch >= '0' && ch <= '7';};
        auto IsHexDigit = [](char ch){return std::isxdigit((unsigned char)ch);};

        while (cur < end)
        {
            if (*cur != '\\')
            {
                if (!bool(flags & UnescapeFlags::strip_cr_bytes) || *cur != '\r')
                    *output_iter++ = *cur;
                cur++;
            }
            else
            {
                cur++; // Skip `/`.
                if (cur == end)
                    Program::Error("Unfinished escape sequence at the end of string.");

                switch (*cur++)
                {
                    // Don't handle `\?`, because it's stupid.
                    case '\'': *output_iter++ = '\''; break;
                    case '\"': *output_iter++ = '\"'; break;
                    case '\\': *output_iter++ = '\\'; break;
                    case 'a': *output_iter++ = '\a'; break;
                    case 'b': *output_iter++ = '\b'; break;
                    case 'f': *output_iter++ = '\f'; break;
                    case 'n': *output_iter++ = '\n'; break;
                    case 'r': *output_iter++ = '\r'; break;
                    case 't': *output_iter++ = '\t'; break;
                    case 'v': *output_iter++ = '\v'; break;

                  default:
                    {
                        cur--;
                        if (!IsOctalDigit(*cur))
                            Program::Error("Invalid escape sequence: `\\", *cur, "`.");

                        ReadSeveralSymbols(3, IsOctalDigit);

                        unsigned int value = 0;
                        std::sscanf(buffer.data(), "%o", &value);
                        if (value > 255)
                            Program::Error("Octal escape sequence with a value larger than 255.");

                        *output_iter++ = char(value);
                    }
                    break;

                  case 'x':
                    {
                        if (ReadSeveralSymbols(2, IsHexDigit) == 0)
                            Program::Error("Expected at least one hex digit after `\\x`");

                        unsigned int value = 0;
                        std::sscanf(buffer.data(), "%x", &value);

                        *output_iter++ = char(value);
                    }
                    break;

                  case 'u':
                    {
                        if (ReadSeveralSymbols(4, IsHexDigit) != 4)
                            Program::Error("Expected 4 hex digits after `\\u`");

                        unsigned int value = 0;
                        std::sscanf(buffer.data(), "%x", &value);
                        char output_buf[Unicode::max_char_len];
                        int output_len = Unicode::Encode(value, output_buf);
                        for (int i = 0; i < output_len; i++)
                            *output_iter++ = output_buf[i];
                    }
                    break;

                  case 'U':
                    {
                        if (ReadSeveralSymbols(8, IsHexDigit) != 8)
                            Program::Error("Expected 8 hex digits after `\\U`");

                        unsigned int value = 0;
                        std::sscanf(buffer.data(), "%x", &value);

                        if (!Unicode::IsValidCharacterCode(value))
                            Program::Error("Unicode codepoint specified in the escape sequence is too large.");

                        char output_buf[Unicode::max_char_len];
                        int output_len = Unicode::Encode(value, output_buf);
                        for (int i = 0; i < output_len; i++)
                            *output_iter++ = output_buf[i];
                    }
                    break;
                }
            }
        }
    }
    [[nodiscard]] inline std::string Unescape(std::string_view str, UnescapeFlags flags = UnescapeFlags::no_flags)
    {
        std::string ret;
        Unescape(str, std::back_inserter(ret), flags);
        return ret;
    }
}
