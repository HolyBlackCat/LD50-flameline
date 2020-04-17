#pragma once

#include <cstddef>
#include <limits>
#include <type_traits>

namespace BitManip
{
    // Returns true if `value` is a non-negative power of two, or 0.
    template <typename T> [[nodiscard]] constexpr bool IsPowerOfTwoOrZero(T value)
    {
        static_assert(std::is_integral_v<T>);

        if constexpr (std::is_signed_v<T>)
            if (value == std::numeric_limits<T>::min())
                return false;

        return (value & (value - 1)) == 0;
    }

    // Returns `log2(value)`, truncated.
    // For non-positive values returns 0.
    template <typename T> [[nodiscard]] constexpr T Log2Truncated(T value)
    {
        if constexpr (std::is_signed_v<T>)
            if (value < 0)
                return 0;

        std::size_t i = sizeof(T) * 8 - std::size_t(std::is_signed_v<T>);
        while (i-- > 0)
        {
            if (value & (T(1) << i))
                return i;
        }

        return 0;
    }

    // Returns the largest power of two less or equal to `value` (but never less than 1).
    template <typename T> [[nodiscard]] constexpr T RoundDownToPositivePowerOfTwo(T value)
    {
        return T(1) << Log2Truncated(value);
    }
}
