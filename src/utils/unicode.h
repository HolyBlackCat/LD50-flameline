#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>

namespace Unicode
{
    using Char = std::uint32_t;

    // A placeholder value for invalid characters.
    inline constexpr Char default_char = 0xfffd;

    // Max bytes per character.
    inline constexpr int max_char_len = 4;

    // Given a byte, checks if it's the first byte of a multibyte character, or is a single-byte character.
    // Even if this function returns true, `byte` can be an invalid first byte.
    // To check for the byte validity, use `FirstByteToCharacterLength`.
    [[nodiscard]] inline bool IsFirstByte(char byte)
    {
        return (byte & 0b11000000) != 0b10000000;
    }

    // Given the first byte of a multibyte character (or a single-byte character), returns the amount of bytes occupied by the character.
    // Returns 0 if this is not a valid first byte, or not a first byte at all.
    [[nodiscard]] inline int FirstByteToCharacterLength(char first_byte)
    {
        if ((first_byte & 0b10000000) == 0b00000000) return 1; // Note the different bit pattern in this one.
        if ((first_byte & 0b11100000) == 0b11000000) return 2;
        if ((first_byte & 0b11110000) == 0b11100000) return 3;
        if ((first_byte & 0b11111000) == 0b11110000) return 4;
        return 0;
    }

    // Returns true if `ch` is a valid unicode ch (aka 'codepoint').
    [[nodiscard]] inline bool IsValidCharacterCode(Char ch)
    {
        return ch <= 0x10ffff;
    }

    // Returns the amount of bytes needed to represent a character.
    // If the character is invalid (use `IsValidCharacterCode` to check for validity) returns 4, which is the maximum possible length
    [[nodiscard]] inline int CharacterCodeToLength(Char ch)
    {
        if (ch <= 0x7f) return 1;
        if (ch <= 0x7ff) return 2;
        if (ch <= 0xffff) return 3;
        // Here `ch <= 0x10ffff`, or the character is invalid.
        // Mathematically the cap should be `0x1fffff`, but Unicode defines the max value to be lower.
        return 4;
    }

    // Encodes a character into UTF8.
    // The minimal buffer length can be determined with `CharacterCodeToLength`.
    // If the character is invalid, writes `default_char` instead.
    // No null-terminator is added.
    // Returns the amount of bytes written, equal to what `CharacterCodeToLength` would return.
    inline int Encode(Char ch, char *buffer)
    {
        if (!IsValidCharacterCode(ch))
            return Encode(default_char, buffer);

        int len = CharacterCodeToLength(ch);
        switch (len)
        {
          case 1:
            *buffer = ch;
            break;
          case 2:
            *buffer++ = 0b11000000 | (ch >> 6);
            *buffer   = 0b10000000 | (ch & 0b00111111);
            break;
          case 3:
            *buffer++ = 0b11100000 |  (ch >> 12);
            *buffer++ = 0b10000000 | ((ch >>  6) & 0b00111111);
            *buffer   = 0b10000000 | ( ch        & 0b00111111);
            break;
          case 4:
            *buffer++ = 0b11110000 |  (ch >> 18);
            *buffer++ = 0b10000000 | ((ch >> 12) & 0b00111111);
            *buffer++ = 0b10000000 | ((ch >> 6 ) & 0b00111111);
            *buffer   = 0b10000000 | ( ch        & 0b00111111);
            break;
        }

        return len;
    }

    // Same as `int Encode(Char ch, char *buffer)`, but appends the data to a string.
    inline int Encode(Char ch, std::string &str)
    {
        char buf[max_char_len];
        int len = Encode(ch, buf);
        str.append(buf, len);
        return len;
    }

    // Decodes a UTF8 character.
    // Returns a pointer to the first byte of the next character.
    // If `end` is not null, it'll stop reading at `end`. In this case `end` will be returned.
    [[nodiscard]] inline const char *FindNextCharacter(const char *data, const char *end = 0)
    {
        do
            data++;
        while (data != end && !IsFirstByte(*data));

        return data;
    }

    // Returns a decoded character or `default_char` on failure.
    // If `end` is not null, it won't attempt to read past it.
    // If `next_char` is not null, it will be set to point to the next byte after the current character.
    // If `data == end`, returns '\0'. (If `end != 0` and `data > end`, also returns '\0'.)
    // If `data == 0`, returns '\0'.
    inline Char Decode(const char *data, const char *end = 0, const char **next_char = 0)
    {
        // Stop if `data` is a null pointer.
        if (!data)
        {
            if (next_char)
                *next_char = 0;
            return 0;
        }

        // Stop if we have an empty string.
        if (end && data >= end) // For `data >= end` to be well-defined, `end` has to be not null if `data` is not null.
        {
            if (next_char)
                *next_char = data;
            return 0;
        }

        // Get character length.
        int len = FirstByteToCharacterLength(*data);

        // Stop if this is not a valid first byte.
        if (len == 0)
        {
            if (next_char)
                *next_char = FindNextCharacter(data, end);
            return default_char;
        }

        // Handle single byte characters.
        if (len == 1)
        {
            if (next_char)
                *next_char = data+1;
            return (unsigned char)*data;
        }

        // Stop if there is not enough characters left in `data`.
        if (end && end - data < len)
        {
            if (next_char)
                *next_char = end;
            return default_char;
        }

        // Extract bits from the first byte.
        Char ret = (unsigned char)*data & (0xff >> len); // `len + 1` would have the same effect as `len`, but it's longer to type.

        // For each remaining byte...
        for (int i = 1; i < len; i++)
        {
            // Stop if it's a first byte of some character.
            if (IsFirstByte(data[i]))
            {
                if (next_char)
                    *next_char = data + i;
                return default_char;
            }

            // Extract bits and append them to the code.
            ret = ret << 6 | ((unsigned char)data[i] & 0b00111111);
        }

        // Get next character position.
        if (next_char)
            *next_char = data + len;

        return ret;
    }


    class Iterator
    {
        const char *cur = 0;
        const char *next = 0;
        const char *range_end = 0;
        Char ch = 0;

      public:
        Iterator() {}
        Iterator(const char *from, const char *to = 0) : next(from), range_end(to)
        {
            ++(*this);
        }
        Iterator(const std::string &str) : Iterator(&*str.begin(), &*str.end()) {}
        Iterator(std::string_view str) : Iterator(&*str.begin(), &*str.end()) {}

        Iterator begin() const
        {
            return *this;
        }
        Iterator end() const
        {
            return {};
        }

        using difference_type   = std::ptrdiff_t;
        using value_type        = Char;
        using pointer           = const Char *;
        using reference         = const Char &;
        using iterator_category = std::forward_iterator_tag;

        Iterator &operator++()
        {
            // Detect end of range if `range_end` is not null.
            if (range_end && next >= range_end)
                next = 0;

            cur = next;
            ch = Decode(cur, range_end, &next);

            // Detect end of range if `range_end` is null.
            if (!range_end && ch == 0)
                cur = next = 0;

            return *this;
        }
        Iterator operator++(int)
        {
            Iterator ret = *this;
            ++(*this);
            return ret;
        }

        reference operator*() const
        {
            return ch;
        }
        pointer operator->() const
        {
            return &ch;
        }

        bool operator==(const Iterator &other) const
        {
            return cur == other.cur;
        }
        bool operator!=(const Iterator &other) const
        {
            return !(*this == other);
        }
    };
}
