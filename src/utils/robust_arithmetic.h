#pragma once

#include <limits>
#include <type_traits>

namespace Robust
{
    template <typename A, typename B>
    [[nodiscard]] constexpr bool equal(A a, B b)
    {
        static_assert(std::is_integral_v<A> && std::is_integral_v<B>, "Parameters must be integral.");

        // This picks the larger of two types. If they have the same size but different signedness, the unsigned one is used.
        using C = std::common_type_t<A, B>;

        if constexpr (std::is_signed_v<A> && !std::is_signed_v<B>)
            return a >= A{} && C(a) == C(b);
        else if constexpr (!std::is_signed_v<A> && std::is_signed_v<B>)
            return b >= B{} && C(a) == C(b);
        else
            return a == b;
    }

    template <typename A, typename B>
    [[nodiscard]] constexpr bool not_equal(A a, B b)
    {
        return !equal(a, b);
    }


    template <typename A, typename B>
    [[nodiscard]] constexpr bool less(A a, B b)
    {
        static_assert(std::is_integral_v<A> && std::is_integral_v<B>, "Parameters must be integral.");

        // This picks the larger of two types. If they have the same size but different signedness, the unsigned one is used.
        using C = std::common_type_t<A, B>;

        if constexpr (std::is_signed_v<A> && !std::is_signed_v<B>)
            return a < A{} || C(a) < C(b);
        else if constexpr (!std::is_signed_v<A> && std::is_signed_v<B>)
            return b > B{} && C(a) < C(b);
        else
            return a < b;
    }

    template <typename A, typename B>
    [[nodiscard]] constexpr bool greater(A a, B b)
    {
        return less(b, a);
    }

    template <typename A, typename B>
    [[nodiscard]] constexpr bool less_eq(A a, B b)
    {
        return !less(b, a);
    }

    template <typename A, typename B>
    [[nodiscard]] constexpr bool greater_eq(A a, B b)
    {
        return !less(a, b);
    }


    // Returns true if `value` can be represented as `A`.
    template <typename A, typename B>
    [[nodiscard]] constexpr bool representable_as(B value)
    {
        return equal(value, A(value));
    }
}
