#pragma once

#include <cstddef>
#include <string>

#include "utils/input_stream.h"

namespace Refl::Parsing
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
                            {
                                input.SkipOne();
                                break;
                            }
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
                            {
                                input.SkipOne();
                                break;
                            }
                            prev = ch;
                            input.SkipOne();
                        }
                    }
                }
            }

            if (!have_whitespace && !have_comment)
                break;

            first = false;
        }

        return !first;
    }
}
