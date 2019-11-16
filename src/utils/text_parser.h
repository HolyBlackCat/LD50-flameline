#pragma once

#include <memory>
#include <string>
#include <utility>

#include "program/errors.h"
#include "utils/check.h"
#include "utils/input_stream.h"
#include "utils/strings.h"
#include "utils/unicode.h"

namespace Stream
{
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
        void ThrowExpected(const Strings::CharCategory &category)
        {
            Program::Error(GetExceptionPrefix() + "Expected " + category.name() + ".");
        }
        void ThrowExpected(const char *str)
        {
            Program::Error(GetExceptionPrefix() + "Expected `" + str + "`.");
        }
        void ThrowExpected(const std::string &str)
        {
            Program::Error(GetExceptionPrefix() + "Expected `" + str + "`.");
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

        // ** Actual parsing functions.

        // Skip one character, return false on failure.
        bool SkipIfPresent(char ch, std::string *append_to = 0)
        {
            if (!MoreData() || PeekChar() != ch)
                return false;
            SkipChar();
            if (append_to)
                *append_to += ch;
            return true;
        }
        bool SkipIfPresent(const Strings::CharCategory &category, std::string *append_to = 0)
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
        bool SkipIfPresent(const char *str, std::string *append_to = 0)
        {
            auto pos = stream->Position();
            while (*str)
            {
                if (!MoreData() || ReadChar() != *str)
                {
                    stream->Seek(pos, Stream::absolute);
                    return false;
                }
                str++;
            }
            if (append_to)
                *append_to += str;
            return true;
        }
        bool SkipIfPresent(const std::string &str, std::string *append_to = 0)
        {
            return SkipIfPresent(str.c_str(), append_to);
        }

        // Skips one character, throws on failure.
        template <typename T>
        auto/*void*/ SkipOne(T &&param, std::string *append_to = 0) -> decltype(SkipIfPresent(std::forward<T>(param)), void())
        {
            if (!SkipIfPresent(std::forward<T>(param), append_to))
                ThrowExpected(param);
        }

        // Skips any amount of characters, returns the amount of characters skipped. Throws on failure.
        template <typename T>
        auto/*int*/ SkipAny(T &&param, std::string *append_to = 0) -> decltype(SkipIfPresent(param/*sic*/), int())
        {
            int count = 0;
            while (SkipIfPresent(param, append_to)) // No forwarding because we're in a loop.
                count++;
            return count;
        }

        // Skips at least one character, returns the amount of characters skipped. Throws on failure.
        template <typename T>
        auto/*int*/ SkipAtLeastOne(T &&param, std::string *append_to = 0) -> decltype(SkipIfPresent(std::forward<T>(param)), int())
        {
            int count = SkipAny(std::forward<T>(param), append_to);
            if (count == 0)
                ThrowExpected(param);
            return count;
        }
    };
}
