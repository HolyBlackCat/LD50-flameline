#pragma once

#include <cstddef>

#include "utils/range_set.h"

namespace Unicode
{
    using std::uint32_t;

    using CharSet = RangeSet<uint32_t>;

    inline constexpr uint32_t default_char = 0xfffd;

    // Even if this function returns `true`, the `byte` is not necessarily a valid first byte.
    // You need to check return value of `CharacterLength` to make sure it's valid.
    inline bool IsFirstByte(char byte)
    {
        return (byte & 0b1100'0000) != 0b1000'0000;
    }

    // Returns 0 if this is not a valid first byte, or not a first byte at all.
    inline int CharacterLength(char first_byte)
    {
        if ((first_byte & 0b10000000) == 0b00000000) return 1; // Note the different bit pattern in this one.
        if ((first_byte & 0b11100000) == 0b11000000) return 2;
        if ((first_byte & 0b11110000) == 0b11100000) return 3;
        if ((first_byte & 0b11111000) == 0b11110000) return 4;
        return 0;
    }

    // Returns a pointer to the first byte of the next character.
    // If `end` is not null, it'll stop reading at `end`. In this case `end` will be returned.
    inline const char *FindNextCharacter(const char *data, const char *end = 0)
    {
        do
            data++;
        while (data == end || !IsFirstByte(*data));

        return data;
    }

    // Returns a decoded character or `default_char` on failure.
    // If `end` is not null, it won't attempt to read past it.
    // If `next_char` is not null, it will be set to point to the next byte after the current character.
    // If `data == end`, returns '\0'.
    inline uint32_t Decode(const char *data, const char *end = 0, const char **next_char = 0)
    {
        // Stop if we have an empty string.
        if (end && data >= end) // For `data >= end` to be well-defined, `end` has to be not null if `data` is not null.
            return 0;

        // Get character length.
        int len = CharacterLength(*data);

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
        uint32_t ret = (unsigned char)*data & (0xff >> len); // `len + 1` would have the same effect as `len`, but it's longer to type.

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
            ret = ret << 6 | ((unsigned char)data[i] & 0b0011'1111);
        }

        // Get next character position.
        if (next_char)
            *next_char = data + len;

        return ret;
    }
}
