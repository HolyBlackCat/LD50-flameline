#pragma once

#include <cctype>
#include <cstddef>
#include <memory>
#include <string_view>
#include <string>
#include <type_traits>
#include <utility>
#include <utility>

#include "program/errors.h"
#include "utils/check.h"
#include "utils/input_stream.h"
#include "utils/strings.h"
#include "utils/unicode.h"

namespace Stream
{
    // Character categories.
    namespace Characters
    {
        // A generic category.
        struct Generic
        {
            [[nodiscard]] virtual bool operator()(char ch) const = 0;
            [[nodiscard]] virtual const char *name() const = 0;
        };

        // A generic character category.
        // Usage: `Is("fancy character", [](char ch){return condition;})`
        template <typename F, CHECK(std::is_convertible_v<decltype(std::declval<F>()(char())), bool>)>
        class Is final : public Generic
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


        // Some character categories.

        #define CHAR_CATEGORY(class_name_, string_, expr_) \
            struct class_name_ final : Generic \
            { \
                [[nodiscard]] bool operator()(char ch) const override {return expr_;} \
                [[nodiscard]] virtual const char *name() const override {return string_;} \
            };

        // Character categories corresponding to the functions from `<cctype>`:

        #define CHAR_CATEGORY_STD(class_name_, func_, string_) CHAR_CATEGORY(class_name_, string_, std::func_((unsigned char)ch))
        // 0-31, 127
        CHAR_CATEGORY_STD( IsControl      , iscntrl  , "a control character"     )
        // !IsControl
        CHAR_CATEGORY_STD( IsNotControl   , isprint  , "a non-control character" )
        // space, \r, \n, \t, \v (vertical tab), \f (form feed)
        CHAR_CATEGORY_STD( IsWhitespace   , isspace  , "a whitespace"            )
        // space, \t
        CHAR_CATEGORY_STD( IsSpaceOrTab   , isblank  , "a space or a tab"        )
        // !IsControl and not a space
        CHAR_CATEGORY_STD( IsVisible      , isgraph  , "a visible character"     )
        // a-z,A-Z
        CHAR_CATEGORY_STD( IsAlpha        , isalpha  , "a letter"                )
        // 0-9
        CHAR_CATEGORY_STD( IsDigit        , isdigit  , "a digit"                 )
        // 0-9,a-f,A-F
        CHAR_CATEGORY_STD( IsHexDigit     , isxdigit , "a hexadecimal digit"     )
        // IsAlpha || IsDigit
        CHAR_CATEGORY_STD( IsAlphaOrDigit , isalnum  , "a letter or a digit"     )
        // IsVisible && !IsAlphaOrDigit
        CHAR_CATEGORY_STD( IsPunctuation  , ispunct  , "a punctuation character" )
        // A-Z
        CHAR_CATEGORY_STD( IsUppercase    , isupper  , "an uppercase letter"     )
        // a-z
        CHAR_CATEGORY_STD( IsLowercase    , islower  , "a lowercase letter"      )
        #undef CHAR_CATEGORY_STD

        // Other categories.
        CHAR_CATEGORY(IsPartOfInteger, "an integer", IsAlphaOrDigit{}(ch) || ch == '+' || ch == '-')
        CHAR_CATEGORY(IsPartOfReal, "a real number", IsPartOfInteger{}(ch) || ch == '.')

        #undef CHAR_CATEGORY


        // Fancy stateful character categories.

        // Matches a c-style identifier.
        class SeqIdentifier final : public Generic
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

    class TextParser
    {
        // A wrapper around `Stream::Input` that simplifies text parsing.
        // May or may not own the stream.

        std::unique_ptr<Input> stream_storage;
        Input *stream = 0; // May or may not point to `stream_storage`.

      public:
        TextParser() {}

        // Stores a reference to the stream.
        TextParser(Input &stream) : stream(&stream) {}

        // Moves the stream into the parser.
        TextParser(Input &&stream) : stream_storage(std::make_unique<Input>(std::move(stream))), stream(stream_storage.get()) {}

        [[nodiscard]] explicit operator bool() const
        {
            return bool(stream);
        }

        // ** Utility functions.
        [[nodiscard]]       Input &Stream()       {return *stream;}
        [[nodiscard]] const Input &Stream() const {return *stream;}

        [[nodiscard]] std::string GetExceptionPrefix() const
        {
            return stream->GetExceptionPrefix() + "Parsing error: ";
        }
        void ThrowExpected(char ch)
        {
            Program::Error(GetExceptionPrefix() + "Expected `" + ch + "`.");
        }
        void ThrowExpected(const Characters::Generic &category)
        {
            Program::Error(GetExceptionPrefix() + "Expected " + category.name() + ".");
        }
        void ThrowExpected(std::string_view str)
        {
            Program::Error((GetExceptionPrefix() + "Expected `").append(str) + "`.");
        }

        // ** Functions forwarded from the underlying stream.
        [[nodiscard]] bool MoreData() const
        {
            return stream->MoreData();
        }
        [[nodiscard]] char PeekChar()
        {
            return stream->PeekByte();
        }
        void SkipChar()
        {
            stream->SkipByte();
        }
        [[nodiscard]] char ReadChar()
        {
            return stream->ReadByte();
        }
        [[nodiscard]] Unicode::Char ReadUnicodeChar()
        {
            return stream->ReadUnicodeChar();
        }

        // ** Actual parsing function.

        // Skips characters.
        // The first parameter controls which characters are skipped. It can be one of:
        // * A single character, which is matched exactly.
        // * A character category, which matches a single character belonging to it.
        // * A string view, which matches its entire contents.
        // If the second parameter is present, the skipped characters are appended to it.
        // The suffix controls how many items are skipped.
        // * Any        - any amount, returns `int`.
        // * IfPresent  - one or none, returns `bool`.
        // * One        - exactly one, returns `void`.
        // * AtLeastOne - one or more, returns `int`.
        // Returns the amount of items skipped. If there is not enough items, throws.

        bool SkipIfPresent(char ch, std::string *append_to = 0)
        {
            if (!MoreData() || PeekChar() != ch)
                return false;
            SkipChar();
            if (append_to)
                *append_to += ch;
            return true;
        }
        bool SkipIfPresent(const Characters::Generic &category, std::string *append_to = 0)
        {
            if (!MoreData())
                return false;
            char ch = PeekChar();
            if (!category(ch))
                return false;
            SkipChar();
            if (append_to)
                *append_to += ch;
            return true;
        }
        bool SkipIfPresent(std::string_view str, std::string *append_to = 0)
        {
            auto pos = stream->Position();
            for (std::size_t i = 0; i < str.size(); i++)
            {
                if (!MoreData() || ReadChar() != str[i])
                {
                    stream->Seek(pos, Stream::absolute);
                    return false;
                }
            }
            if (append_to)
                *append_to += str;
            return true;
        }

        template <typename T>
        auto/*void*/ SkipOne(T &&param, std::string *append_to = 0) -> decltype(SkipIfPresent(std::forward<T>(param)), void())
        {
            if (!SkipIfPresent(std::forward<T>(param), append_to))
                ThrowExpected(param);
        }

        template <typename T>
        auto/*int*/ SkipAny(T &&param, std::string *append_to = 0) -> decltype(SkipIfPresent(std::forward<T>(param)), int())
        {
            int count = 0;
            while (SkipIfPresent(param, append_to)) // No forwarding because we're in a loop.
                count++;
            return count;
        }

        template <typename T>
        auto/*int*/ SkipAtLeastOne(T &&param, std::string *append_to = 0) -> decltype(SkipIfPresent(std::forward<T>(param)), int())
        {
            int count = SkipAny(std::forward<T>(param), append_to);
            if (count == 0)
                ThrowExpected(param);
            return count;
        }

        // Reads a sequence of character matching a condition. See the comments on `SkipIfPresent` for the supported conditions.
        // Throws if there is no suitable characters.
        template <typename T>
        auto /*std::string*/ Extract(T &&param) -> decltype(SkipIfPresent(std::forward<T>(param)), std::string())
        {
            std::string ret;
            SkipAtLeastOne(std::forward<T>(param), &ret);
            return ret;
        }
        // Same, but returns an empty string if there are no suitable characters.
        template <typename T>
        auto /*std::string*/ ExtractIfPresent(T &&param) -> decltype(SkipIfPresent(std::forward<T>(param)), std::string())
        {
            std::string ret;
            SkipAny(std::forward<T>(param), &ret);
            return ret;
        }
    };
}
