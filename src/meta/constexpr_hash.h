#pragma once

#include <cstdint>
#include <string_view>

namespace Meta
{
    using hash_t = std::uint32_t;

    // Constexpr implementation of MurmurHash3, adapted from `https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp`.
    [[nodiscard]] constexpr hash_t cexpr_hash(const char *data, std::size_t len, hash_t seed = 0)
    {
        const std::size_t nblocks = len / 4;

        std::uint32_t h1 = seed;

        constexpr std::uint32_t c1 = 0xcc9e2d51;
        constexpr std::uint32_t c2 = 0x1b873593;

        for (std::size_t i = 0; i < nblocks; i++)
        {
            std::uint32_t k1 = (std::uint32_t)(std::uint8_t)data[i*4+0] << 0
                             | (std::uint32_t)(std::uint8_t)data[i*4+1] << 8
                             | (std::uint32_t)(std::uint8_t)data[i*4+2] << 16
                             | (std::uint32_t)(std::uint8_t)data[i*4+3] << 24;

            k1 *= c1;
            k1 = (k1 << 15) | (k1 >> (32 - 15));
            k1 *= c2;

            h1 ^= k1;
            h1 = (h1 << 13) | (h1 >> (32 - 13));
            h1 = h1 * 5 + 0xe6546b64;
        }

        const char *tail = data + nblocks * 4;

        std::uint32_t k1 = 0;

        switch (len & 3)
        {
          case 3:
            k1 ^= (std::uint32_t)(std::uint8_t)tail[2] << 16;
          case 2:
            k1 ^= (std::uint32_t)(std::uint8_t)tail[1] << 8;
          case 1:
            k1 ^= (std::uint32_t)(std::uint8_t)tail[0];
            k1 *= c1;
            k1 = (k1 << 15) | (k1 >> (32 - 15));
            k1 *= c2; h1 ^= k1;
        };

        h1 ^= len;

        h1 ^= h1 >> 16;
        h1 *= 0x85ebca6b;
        h1 ^= h1 >> 13;
        h1 *= 0xc2b2ae35;
        h1 ^= h1 >> 16;
        return h1;
    }

    [[nodiscard]] constexpr hash_t cexpr_hash(std::string_view view, std::uint32_t seed = 0)
    {
        return cexpr_hash(view.data(), view.size(), seed);
    }

    // Some tests:
    // static_assert(Meta::cexpr_hash("abcd", 42) == 3898664396);
    // static_assert(Meta::cexpr_hash("abcde", 42) == 2933533680);
    // static_assert(Meta::cexpr_hash("abcdef", 42) == 2449278475);
    // static_assert(Meta::cexpr_hash("abcdefg", 42) == 1781200409);
}
