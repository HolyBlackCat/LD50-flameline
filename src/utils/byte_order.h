#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ByteOrder
{
    enum Order {little, big};

    inline constexpr Order native =
    #ifdef PLATFORM_LITTLE_ENDIAN
        little;
    #endif
    #ifdef PLATFORM_BIG_ENDIAN
        big;
    #endif

    inline void SwapBytes(std::uint8_t *data, std::size_t len)
    {
        std::size_t half_len = len/2;
        for (std::size_t i = 0; i < half_len; i++)
            std::swap(data[i], data[len-1-i]);
    }

    inline void ConvertBytes(std::uint8_t *data, std::size_t len, Order order)
    {
        if (order != native)
            SwapBytes(data, len);
    }

    template <typename T> void Swap(T &value)
    {
        static_assert(std::is_arithmetic_v<T>, "The parameter has to be arithmetic.");

        SwapBytes(reinterpret_cast<std::uint8_t *>(&value), sizeof(T));
    }

    template <typename T> void Convert(T &value, Order order)
    {
        static_assert(std::is_arithmetic_v<T>, "The parameter has to be arithmetic.");

        if (order != native)
            Swap(value);
    }

    template <typename T> [[nodiscard]] T Little(T value)
    {
        static_assert(std::is_arithmetic_v<T>, "The parameter has to be arithmetic.");
        Convert(value, little);
        return value;
    }
    template <typename T> [[nodiscard]] T Big(T value)
    {
        static_assert(std::is_arithmetic_v<T>, "The parameter has to be arithmetic.");
        Convert(value, big);
        return value;
    }
}
