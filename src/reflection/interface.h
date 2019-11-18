#pragma once

#include <cstring>
#include <cstddef>
#include <string>
#include <type_traits>

#include "utils/input_stream.h"
#include "utils/meta.h"
#include "utils/output_stream.h"

namespace Refl
{
    namespace ParsingUtilities
    {
        // Skips whitespace and c++-style comments (`//` and `/* */`).
        // Returns true if skipped at least one character.
        // Throws if there is an unterminated `/*` comment.
        inline bool SkipWhitespaceAndComments(Stream::Input &input)
        {
            bool first = true;

            while (1)
            {
                if (!input.MoreData())
                    break;

                // Skip any whitespace.
                bool have_whitespace = input.Discard<Stream::any>(Stream::Char::IsWhitespace{});

                bool have_comment = false;

                // Check for a slash.
                if (input.MoreData() && input.PeekChar() == '/')
                {
                    input.SkipOne();
                    char ch = input.MoreData() ? input.PeekChar() : '\0';
                    if (ch != '*' && ch != '/')
                    {
                        // This is not a comment, unget the slash.
                        input.Seek(-1, Stream::relative);
                    }
                    else
                    {
                        // This is a comment, skip it.
                        have_comment = 1;
                        input.SkipOne();
                        if (ch == '/') // One-line comment.
                        {
                            while (input.MoreData())
                            {
                                char ch = input.PeekChar();
                                if (ch == '\r' || ch == '\n')
                                    break;
                                input.SkipOne();
                            }
                        }
                        else // ch == '*', multi-line comment.
                        {
                            auto pos = input.Position();

                            char prev = 0;
                            while (1)
                            {
                                if (!input.MoreData())
                                {
                                    input.Seek(pos-2, Stream::absolute); // Move cursor to the beginning of the comment, for better error reporting.
                                    Program::Error(input.GetExceptionPrefix() + "Unterminated comment.");
                                }
                                char ch = input.PeekChar();
                                if (ch == '/' && prev == '*')
                                    break;
                                prev = ch;
                                input.SkipOne();
                            }
                        }
                        input.SkipOne();
                    }
                }

                if (!have_whitespace && !have_comment)
                    break;

                first = false;
            }

            return !first;
        }
    }

    template <typename T>
    class InterfaceBasic : Meta::with_virtual_destructor<InterfaceBasic<T>>
    {
        static_assert(!std::is_reference_v<T>, "Can't make a reflection interface for a reference.");
        static_assert(!std::is_volatile_v<T>, "Can't make a reflection interface for a volatile type.");

        T *object = 0;

      public:
        constexpr InterfaceBasic() {}
        constexpr InterfaceBasic(T &object) : object(&object) {}

        [[nodiscard]] explicit operator bool() const {return bool(value);}

        [[nodiscard]] constexpr T &value() const {return *object;}


        struct ToStringOptions
        {
            bool multiline = false;
            int indent = 4; // Indentation step.
            int extra_indent = 0; // Extra indentation at the beginning of each line, except the first one. Intended for an internal use.
        };

        virtual void ToString(Stream::Output &str, const ToStringOptions &options = {}) const = 0;

        [[nodiscard]] std::string ToString(const ToStringOptions &options = {}) const
        {
            std::string ret;
            Stream::Output out = Stream::Output::Container(ret);
            ToString(out, options);
            return ret;
        }


        struct FromStringOptions
        {
            bool ignore_missing_fields = false; // If parsing a struct, don't complain if any fields are missing.
        };

        // This should ignore any leading whitespace and comments.
        virtual void FromString(Stream::Input &input, const FromStringOptions &options = {}) = 0;

        // Unlike the version that takes a `Input`, this one throws if the string has any extra data at the end (after any whitespace or comments).
        void FromString(const std::string &string, const FromStringOptions &options = {})
        {
            Stream::Input in = Stream::Input(Stream::ReadOnlyData::mem_reference(string), Stream::LocationStyle::text_position);
            FromString(in, options);
            in.ExpectEnd();
        }
        void FromString(const char *string, const FromStringOptions &options = {})
        {
            Stream::Input in = Stream::Input(Stream::ReadOnlyData::mem_reference(string, string + std::strlen(string)), Stream::LocationStyle::text_position);
            FromString(in, options);
            in.ExpectEnd();
        }
    };
}
