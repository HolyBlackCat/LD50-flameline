#pragma once

#include <limits>
#include <type_traits>

namespace RobustCompare
{
    template <typename A, typename B> constexpr bool int_equal(A a, B b)
    {
        static_assert(std::is_integral_v<A> && std::is_integral_v<B>, "Parametes must be integral.");

        // This picks a larger of two types. If they have the same size but different signedness, the unsigned one is used.
        using C = std::common_type_t<A, B>;

        if (std::is_signed_v<A> && !std::is_signed_v<B>)
            return a >= A{} && static_cast<C>(a) == static_cast<C>(b);
        else if (!std::is_signed_v<A> && std::is_signed_v<B>)
            return b >= B{} && static_cast<C>(a) == static_cast<C>(b);
        else
            return a == b;
    }

    template <typename A, typename B> constexpr bool int_not_equal(A a, B b)
    {
        return !int_equal(a, b);
    }


    template <typename A, typename B> constexpr bool int_less(A a, B b)
    {
        static_assert(std::is_integral_v<A> && std::is_integral_v<B>, "Parametes must be integral.");

        // This picks a larger of two types. If they have the same size but different signedness, the unsigned one is used.
        using C = std::common_type_t<A, B>;

        if (std::is_signed_v<A> && !std::is_signed_v<B>)
            return a < A{} || static_cast<C>(a) < static_cast<C>(b);
        else if (!std::is_signed_v<A> && std::is_signed_v<B>)
            return b > B{} && static_cast<C>(a) < static_cast<C>(b);
        else
            return a < b;
    }

    template <typename A, typename B> constexpr bool int_greater(A a, B b)
    {
        return int_less(b, a);
    }

    template <typename A, typename B> constexpr bool int_less_eq(A a, B b)
    {
        return !int_less(b, a);
    }

    template <typename A, typename B> constexpr bool int_greater_eq(A a, B b)
    {
        return !int_less(a, b);
    }


    template <typename A, typename B, typename C> constexpr bool int_in_inclusive_range(A x, B min, C max)
    {
        return int_less_eq(min, x) && int_less_eq(x, max);
    }

    template <typename A, typename B, typename C> constexpr bool int_not_in_inclusive_range(A x, B min, C max)
    {
        return !int_in_inclusive_range(x, min, max);
    }

    template <typename A, typename B, typename C> constexpr A int_clamp(A x, B min, C max)
    {
        if (int_less(x, min))
            return static_cast<A>(min);
        else if (int_greater(x, max))
            return static_cast<A>(max);
        else
            return x;
    }
}
