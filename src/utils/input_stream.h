#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "program/errors.h"
#include "utils/byte_order.h"
#include "utils/check.h"
#include "utils/escape.h"
#include "utils/memory_access.h"
#include "utils/meta.h"
#include "utils/readonly_data.h"
#include "utils/strings.h"
#include "utils/unicode.h"

namespace Stream
{
    namespace Char
    {
        // A base class for character categories.
        struct Category
        {
            [[nodiscard]] virtual bool operator()(char ch) const = 0;
            [[nodiscard]] virtual std::string name() const = 0;
        };

        // A category matching a single character.
        class EqualTo final : public Category
        {
            char saved_char = 0;

          public:
            EqualTo(char ch) : saved_char(ch) {}

            [[nodiscard]] bool operator()(char ch) const override
            {
                return ch == saved_char;
            }
            [[nodiscard]] std::string name() const override
            {
                return "`" + Strings::Escape(saved_char) + "`";
            }
        };

        // A generic character category.
        // Usage: `Is("fancy character", [](char ch){return condition;})`
        template <typename F, CHECK(std::is_convertible_v<decltype(std::declval<F>()(char())), bool>)>
        class Is final : public Category
        {
            F &&func;
            const char *name_str;

          public:
            Is(const char *name, F &&func) : func(std::move(func)), name_str(name) {}

            [[nodiscard]] bool operator()(char ch) const override
            {
                return func(ch);
            }
            [[nodiscard]] std::string name() const override
            {
                return name_str;
            }
        };


        // Some character categories.

        #define CHAR_CATEGORY(class_name_, string_, expr_) \
            struct class_name_ final : Category \
            { \
                [[nodiscard]] bool operator()(char ch) const override {return expr_;} \
                [[nodiscard]] std::string name() const override {return string_;} \
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

        #undef CHAR_CATEGORY


        // Fancy stateful character categories.

        // Matches a c-style identifier.
        class SeqIdentifier final : public Category
        {
            mutable bool first_char = true;

          public:
            [[nodiscard]] bool operator()(char ch) const override
            {
                bool ok = IsAlpha{}(ch) || ch == '_' || (!first_char && IsDigit{}(ch));
                first_char = false;
                return ok;
            }

            [[nodiscard]] std::string name() const override {return "an identifier";}
        };
    }

    namespace impl
    {
        template <typename T>
        using detect_appendable_byte_seq = decltype(
            std::declval<T&>().push_back(std::uint8_t{}),
            void()
        );

        template <typename T>
        inline constexpr bool is_appendable_byte_seq_v = Meta::is_detected<impl::detect_appendable_byte_seq, T>;

        template <typename T>
        inline constexpr bool is_appendable_byte_seq_ptr_or_null_v = std::is_null_pointer_v<T> || (std::is_pointer_v<T> && is_appendable_byte_seq_v<std::remove_pointer_t<T>>);
    }

    enum PositionCategory
    {
        absolute,
        relative,
        end,
    };

    enum LocationStyle
    {
        none,
        byte_offset,
        text_position,
        text_byte_position,
    };

    enum ExtractMode
    {
        one, // Exactly one.
        any, // Any amount.
        if_present, // If present.
        at_least_one, // At least one.
    };

    class Input
    {
        struct Data
        {
            ReadOnlyData file;
            std::size_t position = 0;
            LocationStyle location_style = none;
        };
        Data data;

        void ThrowIfNoData(std::size_t bytes) const
        {
            if (data.position + bytes > data.file.size())
                Program::Error(GetExceptionPrefix() + "Unexpected end of input.");
        }

      public:
        // Constructs an empty stream.
        Input() {}

        // Attaches a stream to a memory file.
        Input(ReadOnlyData file, LocationStyle location_style = none)
        {
            data.file = std::move(file);
            data.location_style = location_style;
        }

        Input(Input &&other) noexcept : data(std::exchange(other.data, {})) {}
        Input &operator=(Input other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        [[nodiscard]] explicit operator bool() const
        {
            return bool(data.file);
        }

        // Returns a name of the data source the stream is bound to.
        [[nodiscard]] std::string GetTarget() const
        {
            return data.file.name();
        }

        // Returns a string describing current location in the stream.
        // This function can be costly for some location flavors.
        [[nodiscard]] std::string GetLocation() const
        {
            switch (data.location_style)
            {
              case none:
              default:
                return "";
              case byte_offset:
                return Str("offset 0x", std::hex, std::uppercase, data.position);
              case text_position:
              case text_byte_position:
                return Strings::GetSymbolPosition(data.file.data_char(), data.file.data_char() + data.position).ToString();
            }
        }

        // Uses `GetLocationString` to construct a prefix for exception messages.
        [[nodiscard]] std::string GetExceptionPrefix() const
        {
            std::string ret = "In an input stream bound to `" + GetTarget() + "`";

            if (std::string loc = GetLocation(); loc.size() > 0)
            {
                ret += ", at " + loc;
            }

            ret += ":\n";
            return ret;
        }

        // Moves the cursor.
        // Throws if it ends up out of bounds.
        void Seek(std::ptrdiff_t offset, PositionCategory category)
        {
            std::size_t base_pos =
                category == relative ? data.position    :
                category == end      ? data.file.size() : 0;

            std::size_t new_pos = base_pos + offset;

            // Note the `>` rather than `<=`. Pointing to a single byte past the end of the file is allowed.
            // Note that we don't check for an overflow here. It shouldn't be necessary,
            // as `file.size()` will be representable as `ptrdiff_t`, because the file resides in memory.
            if (new_pos > data.file.size())
                Program::Error(GetExceptionPrefix() + "Cursor position is out of bounds.");

            data.position = new_pos;
        }

        // Returns the current byte offset of the cursor.
        [[nodiscard]] std::size_t Position() const
        {
            return data.position;
        }

        // Checks if the stream has more data available at the current cursor position.
        [[nodiscard]] bool MoreData() const
        {
            return data.position < data.file.size();
        }

        // Throws if there is more data available at the current cursor position.
        void ExpectEnd() const
        {
            if (MoreData())
                Program::Error(GetExceptionPrefix() + "Unexpected junk at the end of input.");
        }

        // Returns the next byte, without advancing the cursor.
        [[nodiscard]] std::uint8_t PeekByte() const
        {
            ThrowIfNoData(1);
            return data.file.data()[data.position];
        }
        [[nodiscard]] char PeekChar() const
        {
            return PeekByte();
        }

        // Reads a single byte.
        [[nodiscard]] std::uint8_t ReadByte()
        {
            ThrowIfNoData(1);
            return data.file.data()[data.position++];
        }
        [[nodiscard]] char ReadChar()
        {
            return ReadByte();
        }

        // Reads a single UTF8 character.
        // Shouldn't throw if the encoding is incorrect, instead `Unicode::default_char` will be returned.
        [[nodiscard]] Unicode::Char ReadUnicodeChar()
        {
            std::uint8_t first = ReadByte();
            int len = Unicode::FirstByteToCharacterLength(first);

            // Check if the first byte is valid.
            if (len == 0)
            {
                while (MoreData() && !Unicode::IsFirstByte(PeekByte()))
                    SkipOne();
                return Unicode::default_char;
            }

            // Check if the character occupies a single-byte.
            if (len == 1)
                return first;

            // Extract bits from the first byte.
            Unicode::Char ret = first & (0x7f >> len); // `0xff` would also work.

            // Read remaining bytes.
            for (int i = 1; i < len; i++)
            {
                if (!MoreData())
                    return Unicode::default_char;

                std::uint8_t byte = PeekByte();

                // Make sure it's not a first byte of some character.
                if (Unicode::IsFirstByte(byte))
                    return Unicode::default_char;

                // Now we can safely move the cursor.
                SkipOne();

                // Extract bits from the byte and append them to the result.
                ret = (ret << 6) | (byte & 0b00111111);
            }

            return ret;
        }

        // Reads a sequence of bytes.
        void Read(std::uint8_t *buffer, std::size_t size)
        {
            ThrowIfNoData(size);
            std::copy_n(data.file.data() + data.position, size, buffer);
            data.position += size;
        }
        void Read(char *buffer, std::size_t size)
        {
            Read(reinterpret_cast<std::uint8_t *>(buffer), size);
        }

        // Skips one byte.
        void SkipOne()
        {
            Seek(1, relative);
        }
        // Skips several bytes.
        void Skip(std::size_t count = 1)
        {
            Seek(count, relative);
        }

        // Reads a single arithmetic value with a specified byte order.
        template <typename T>
        [[nodiscard]] T ReadWithByteOrder(ByteOrder::Order order)
        {
            T ret;
            Read(reinterpret_cast<std::uint8_t *>(&ret), sizeof ret);
            ByteOrder::Convert(ret, order);
            return ret;
        }
        template <typename T>
        [[nodiscard]] T ReadLittle()
        {
            return ReadWithByteOrder<T>(ByteOrder::little);
        }
        template <typename T>
        [[nodiscard]] T ReadBig()
        {
            return ReadWithByteOrder<T>(ByteOrder::big);
        }
        template <typename T>
        [[nodiscard]] T ReadNative()
        {
            return ReadWithByteOrder<T>(ByteOrder::native);
        }

        // Reads a sequence of arithmetic values with a specified byte order.
        template <typename T>
        void ReadWithByteOrder(ByteOrder::Order order, T *buffer, std::size_t count)
        {
            Read(reinterpret_cast<std::uint8_t *>(buffer), count * sizeof *buffer);
            for (std::size_t i = 0; i < count; i++)
                ByteOrder::Convert(buffer[i], order);
        }
        template <typename T>
        void ReadLittle(T *buffer, std::size_t count)
        {
            ReadWithByteOrder(ByteOrder::little, buffer, count);
        }
        template <typename T>
        void ReadBig(T *buffer, std::size_t count)
        {
            ReadWithByteOrder(ByteOrder::big, buffer, count);
        }
        template <typename T>
        void ReadNative(T *buffer, std::size_t count)
        {
            ReadWithByteOrder(ByteOrder::native, buffer, count);
        }

        // Reads matching characters from the input.
        // `mode` affects how many characters are read, and whether or not reading 0 characters causes an exception.
        // If `append_to` is not `nullptr`, the matching characters are appended to it.
        // Returns the amount of characters processed.
        template <ExtractMode mode = at_least_one, typename T, CHECK(impl::is_appendable_byte_seq_ptr_or_null_v<T>)>
        std::size_t Extract(const Char::Category &category, T append_to) // `append_to` can be null.
        {
            constexpr bool several = mode == at_least_one || mode == any;
            constexpr bool throw_if_none = mode == at_least_one || mode == one;

            std::size_t count = 0;

            do
            {
                if (!MoreData())
                    break;
                std::uint8_t byte = PeekByte();
                if (!category(byte))
                    break;
                SkipOne();
                if constexpr (!std::is_null_pointer_v<T>)
                    if (append_to)
                        append_to->push_back(byte);
                count++;
            }
            while (several);

            if (throw_if_none && count == 0)
                Program::Error(GetExceptionPrefix() + "Expected " + category.name() + ".");

            return count;
        }
        template <ExtractMode mode = at_least_one, typename T, CHECK(impl::is_appendable_byte_seq_ptr_or_null_v<T>)>
        std::size_t Extract(std::uint8_t byte, T append_to)
        {
            return Extract<mode>(Char::EqualTo(byte), append_to);
        }

        // Reads matching characters from the input.
        // `mode` affects whether or not reading 0 characters causes an exception.
        // Returns the matching characters in a container of type `C`.
        template <ExtractMode mode = at_least_one, typename C = std::string, CHECK(impl::is_appendable_byte_seq_v<C>)>
        [[nodiscard]] C Extract(const Char::Category &category)
        {
            static_assert(mode == at_least_one || mode == any, "Mode has to be `at_least_one` or `any`.");
            C ret;
            Extract<mode>(category, &ret);
            return ret;
        }
        template <ExtractMode mode = at_least_one, typename C = std::string, CHECK(impl::is_appendable_byte_seq_v<C>)>
        [[nodiscard]] C Extract(std::uint8_t byte)
        {
            return Extract<mode, C>(Char::EqualTo(byte));
        }

        // Discards matching characters from the input.
        // `mode` affects how many characters are read, and whether or not reading 0 characters causes an exception.
        // Returns the amount of characters processed.
        template <ExtractMode mode = one>
        std::size_t Discard(const Char::Category &category)
        {
            return Extract<mode>(category, nullptr);
        }
        template <ExtractMode mode = one>
        std::size_t Discard(std::uint8_t byte)
        {
            return Extract<mode>(Char::EqualTo(byte), nullptr);
        }

        // Discards a sequence of bytes from the input.
        // `mode` affects whether it returns `false` on failure or throws.
        template <ExtractMode mode = one>
        bool DiscardBytes(const std::uint8_t *bytes, std::size_t count)
        {
            static_assert(mode == one || mode == if_present, "Mode has to be `one` or `if_present`.");
            auto pos = Position();
            for (std::size_t i = 0; i < count; i++)
            {
                if (!MoreData() || PeekByte() != bytes[i])
                {
                    Seek(pos, absolute);
                    if constexpr (mode == one)
                        Program::Error(GetExceptionPrefix() + "Expected \"" + Strings::Escape(std::string_view(reinterpret_cast<const char *>(bytes), count)) + "\".");
                    return false;
                }
                SkipOne();
            }
            return true;
        }
        template <ExtractMode mode = one>
        bool DiscardChars(const char *chars, std::size_t count)
        {
            return DiscardBytes<mode>(reinterpret_cast<const std::uint8_t *>(chars), count);
        }
        template <ExtractMode mode = one>
        bool DiscardBytes(std::initializer_list<std::uint8_t> list)
        {
            return DiscardBytes<mode>(&*list.begin(), list.size());
        }
        template <ExtractMode mode = one>
        bool DiscardChars(std::string_view view)
        {
            return DiscardChars<mode>(&*view.begin(), view.size());
        }
    };
}
