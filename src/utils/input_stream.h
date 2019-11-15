#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "program/errors.h"
#include "utils/byte_order.h"
#include "utils/memory_access.h"
#include "utils/readonly_data.h"
#include "utils/strings.h"
#include "utils/unicode.h"

namespace Stream
{
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
                Program::Error(GetExceptionPrefix() + "Unexpected end of data.");
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

            ret += ": ";
            return ret;
        }

        // Moves the cursor.
        // Throws if it ends up out of bounds.
        void Seek(std::ptrdiff_t offset, PositionCategory category = relative)
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
                Program::Error(GetExceptionPrefix() + "Expected end of data.");
        }

        // Returns the next byte, without advancing the cursor.
        [[nodiscard]] std::uint8_t PeekByte() const
        {
            ThrowIfNoData(1);
            return data.file.data()[data.position];
        }

        // Reads a single byte.
        [[nodiscard]] std::uint8_t ReadByte()
        {
            ThrowIfNoData(1);
            return data.file.data()[data.position++];
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
                    SkipByte();
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
                SkipByte();

                // Extract bits from the byte and append them to the result.
                ret = (ret << 6) | (byte & 0b00111111);
            }

            return ret;
        }

        // Reads a sequence of bytes.
        void ReadBytes(std::uint8_t *buffer, std::size_t size)
        {
            ThrowIfNoData(size);
            std::copy_n(data.file.data() + data.position, size, buffer);
            data.position += size;
        }
        void ReadBytes(char *buffer, std::size_t size)
        {
            ReadBytes(reinterpret_cast<std::uint8_t *>(buffer), size);
        }

        // Skips several bytes or objects.
        void SkipByte()
        {
            Seek(1);
        }
        void SkipBytes(std::size_t count)
        {
            Seek(count);
        }
        template <typename T>
        void Skip(std::size_t count)
        {
            static_assert(std::is_arithmetic_v<T>, "The template parameter must be arithmetic.");
            SkipBytes(count * sizeof(T));
        }

        // Reads a single arithmetic value with a specified byte order.
        template <typename T>
        [[nodiscard]] T ReadWithByteOrder(ByteOrder::Order order)
        {
            T ret;
            ReadBytes(reinterpret_cast<std::uint8_t *>(&ret), sizeof ret);
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
            ReadBytes(reinterpret_cast<std::uint8_t *>(buffer), count * sizeof *buffer);
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
    };
}
