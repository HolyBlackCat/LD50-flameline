#pragma once

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <string>
#include <type_traits>
#include <utility>

#include "utils/check.h"
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


    // A character category.
    struct CharCategory
    {
        [[nodiscard]] virtual bool operator()(char ch) const = 0;
        [[nodiscard]] virtual const char *name() const = 0;
    };

    // A generic character category.
    // Usage: `Is("fancy character", [](char ch){return condition;})`
    template <typename F, CHECK(std::is_convertible_v<decltype(std::declval<F>()(char())), bool>)>
    class Is final : public CharCategory
    {
        F &&func;
        const char *name_str;

      public:
        Is(const char *name, F &&func) : func(std::move(func)), name_str(name) {}

        bool operator()(char ch) const override
        {
            return func(ch);
        }
        const char *name() const override
        {
            return name_str;
        }
    };


    // Character categories corresponding to the functions from `<cctype>`:

    #define CHAR_CATEGORY(class_name_, func_, string_) \
        struct class_name_ final : CharCategory \
        { \
            [[nodiscard]] bool operator()(char ch) const override {return std::func_((unsigned char)ch);} \
            [[nodiscard]] virtual const char *name() const override {return string_;} \
        };

    // 0-31, 127
    CHAR_CATEGORY( IsControl      , iscntrl  , "a control character"     )
    // !IsControl
    CHAR_CATEGORY( IsNotControl   , isprint  , "a non-control character" )
    // space, \r, \n, \t, \v (vertical tab), \f (form feed)
    CHAR_CATEGORY( IsWhitespace   , isspace  , "a whitespace"            )
    // space, \t
    CHAR_CATEGORY( IsSpaceOrTab   , isblank  , "a space or a tab"        )
    // !IsControl and not a space
    CHAR_CATEGORY( IsVisible      , isgraph  , "a visible character"     )
    // a-z,A-Z
    CHAR_CATEGORY( IsAlpha        , isalpha  , "a letter"                )
    // 0-9
    CHAR_CATEGORY( IsDigit        , isdigit  , "a digit"                 )
    // 0-9,a-f,A-F
    CHAR_CATEGORY( IsHexDigit     , isxdigit , "a hexadecimal digit"     )
    // IsAlpha || IsDigit
    CHAR_CATEGORY( IsAlphaOrDigit , isalnum  , "a letter or a digit"     )
    // IsVisible && !IsAlphaOrDigit
    CHAR_CATEGORY( IsPunctuation  , ispunct  , "a punctuation character" )
    // A-Z
    CHAR_CATEGORY( IsUppercase    , isupper  , "an uppercase letter"     )
    // a-z
    CHAR_CATEGORY( IsLowercase    , islower  , "a lowercase letter"      )

    #undef CHAR_CATEGORY


    // Fancy stateful character categories.

    // Matches a c-style identifier.
    class SeqIdentifier final : public CharCategory
    {
        mutable bool first_char = true;

      public:
        bool operator()(char ch) const override
        {
            bool ok = IsAlpha{}(ch) || ch == '_' || (!first_char && IsDigit{}(ch));
            first_char = false;
            return ok;
        }

        const char *name() const override {return "an identifier";}
    };
}

using Strings::Str;
