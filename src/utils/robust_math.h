#pragma once

#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include "meta/misc.h"

/* The functions in this file can be used for ROBUST comparsions between integral and/or floating-point types.
 *
 * About reliability:
 * The `float x float` comparsions should be completely reliable, as we simply use the builtin comparsion operators.
 * The `int x int` comparsions should also be reliable, as the comparsion algorithm is simple.
 * The `int x float` comparsions, on the other hand, rely on a complicated algorithm. Even though they were tested,
 *     it's hard to guarantee complete robustness here. Also they might be slow.
 */

namespace Robust
{
    // A partial ordering.
    // "Partial" means that some values are not comparable (i.e. NaNs, that's what `unordered` is for).
    enum class Ordering {less, equal, greater, unordered};

    // This set of operators allows comparsions between an `Ordering` and 0.
    // `ordering < 0` -> less
    // `ordering > 0` -> greater
    // `ordering <= 0` -> less or equal
    // `ordering >= 0` -> greater or equal
    // `ordering == 0` -> equal
    // `ordering != 0` -> not equal
    // If the ordering is `unordered`, then only `!= 0` returns true.
    // The operands can be swapped if that looks better to you.
    [[nodiscard]] inline bool operator<(Ordering o, decltype(nullptr)) {return o == Ordering::less;}
    [[nodiscard]] inline bool operator>(Ordering o, decltype(nullptr)) {return o == Ordering::greater;}
    [[nodiscard]] inline bool operator<=(Ordering o, decltype(nullptr)) {return o == Ordering::less || o == Ordering::equal;}
    [[nodiscard]] inline bool operator>=(Ordering o, decltype(nullptr)) {return o == Ordering::greater || o == Ordering::equal;}
    [[nodiscard]] inline bool operator==(Ordering o, decltype(nullptr)) {return o == Ordering::equal;}
    [[nodiscard]] inline bool operator!=(Ordering o, decltype(nullptr)) {return o != Ordering::equal;}
    [[nodiscard]] inline bool operator<(decltype(nullptr), Ordering o) {return o > 0;}
    [[nodiscard]] inline bool operator>(decltype(nullptr), Ordering o) {return o < 0;}
    [[nodiscard]] inline bool operator<=(decltype(nullptr), Ordering o) {return o >= 0;}
    [[nodiscard]] inline bool operator>=(decltype(nullptr), Ordering o) {return o <= 0;}
    [[nodiscard]] inline bool operator==(decltype(nullptr), Ordering o) {return o == 0;}
    [[nodiscard]] inline bool operator!=(decltype(nullptr), Ordering o) {return o != 0;}


    namespace impl
    {
        // Compares an integral and a floating-point value.
        // Despite the parameter names, it doesn't matter which one is which.
        // Follows a so-called 'partial ordering': for some pairs of values you get a special 'undefined' result (i.e. for NaNs compared with any number).
        template <typename I, typename F>
        [[nodiscard]] Ordering compare_int_float_three_way(I i, F f)
        {
            if constexpr (std::is_integral_v<F> && std::is_floating_point_v<I>)
            {
                return compare_int_float_three_way(f, i);
            }
            else
            {
                static_assert(std::is_integral_v<I> && std::is_floating_point_v<F>);
                static_assert(std::numeric_limits<F>::radix == 2);

                // This should be exactly representable as F due to being a power of two.
                constexpr F I_min_as_F = std::numeric_limits<I>::min();

                // The `numeric_limits<I>::max()` itself might not be representable as F, so we use this instead.
                constexpr F I_max_as_F_plus_1 = F(std::numeric_limits<I>::max()/2+1) * 2;

                // Check if the constants above overflowed to infinity. Normally this shouldn't happen.
                constexpr bool limits_overflow = I_min_as_F * 2 == I_min_as_F || I_max_as_F_plus_1 * 2 == I_max_as_F_plus_1;
                if constexpr (limits_overflow)
                {
                    // Manually check for special floating-point values.
                    if (std::isinf(f))
                        return f > 0 ? Ordering::less : Ordering::greater;
                    if (std::isnan(f))
                        return Ordering::unordered;
                }

                if (limits_overflow || f >= I_min_as_F)
                {
                    // `f <= I_max_as_F_plus_1 - 1` would be problematic due to rounding, so we use this instead.
                    if (limits_overflow || f - I_max_as_F_plus_1 <= -1)
                    {
                        I f_trunc = f;
                        if (f_trunc < i)
                            return Ordering::greater;
                        if (f_trunc > i)
                            return Ordering::less;

                        F f_frac = f - f_trunc;
                        if (f_frac < 0)
                            return Ordering::greater;
                        if (f_frac > 0)
                            return Ordering::less;

                        return Ordering::equal;
                    }

                    return Ordering::less;
                }

                if (f < 0)
                    return Ordering::greater;

                return Ordering::unordered;
            }
        }
    }


    template <typename A, typename B>
    [[nodiscard]] constexpr bool equal(A a, B b)
    {
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be arithmetic.");

        constexpr bool a_is_int = std::is_integral_v<A>;
        constexpr bool b_is_int = std::is_integral_v<B>;

        if constexpr (!a_is_int && !b_is_int)
        {
            return a == b;
        }
        else if constexpr (a_is_int != b_is_int)
        {
            return impl::compare_int_float_three_way(a, b) == Ordering::equal;
        }
        else // a_is_int && b_is_int
        {
            // This picks the larger of two types. If they have the same size but different signedness, the unsigned one is used.
            using C = std::common_type_t<A, B>;

            if constexpr (std::is_signed_v<A> && !std::is_signed_v<B>)
                return a >= A{} && C(a) == C(b);
            else if constexpr (!std::is_signed_v<A> && std::is_signed_v<B>)
                return b >= B{} && C(a) == C(b);
            else
                return a == b;
        }
    }

    template <typename A, typename B>
    [[nodiscard]] constexpr bool not_equal(A a, B b)
    {
        // We could remove the `static_assert`, but then the error messages would get longer.
        // Also, in theory, ADL could kick in if a class is passed.
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be arithmetic.");
        return !equal(a, b);
    }


    template <typename A, typename B>
    [[nodiscard]] constexpr bool less(A a, B b)
    {
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be arithmetic.");

        constexpr bool a_is_int = std::is_integral_v<A>;
        constexpr bool b_is_int = std::is_integral_v<B>;

        if constexpr (!a_is_int && !b_is_int)
        {
            return a < b;
        }
        else if constexpr (a_is_int != b_is_int)
        {
            return impl::compare_int_float_three_way(a, b) == Ordering::less;
        }
        else // a_is_int && b_is_int
        {
            // This picks the larger of two types. If they have the same size but different signedness, the unsigned one is used.
            using C = std::common_type_t<A, B>;

            if constexpr (std::is_signed_v<A> && !std::is_signed_v<B>)
                return a < A{} || C(a) < C(b);
            else if constexpr (!std::is_signed_v<A> && std::is_signed_v<B>)
                return b > B{} && C(a) < C(b);
            else
                return a < b;
        }
    }

    template <typename A, typename B>
    [[nodiscard]] constexpr bool greater(A a, B b)
    {
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be arithmetic.");
        return less(b, a);
    }

    template <typename A, typename B>
    [[nodiscard]] constexpr bool less_eq(A a, B b)
    {
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be arithmetic.");
        return !less(b, a);
    }

    template <typename A, typename B>
    [[nodiscard]] constexpr bool greater_eq(A a, B b)
    {
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be arithmetic.");
        return !less(a, b);
    }


    template <typename A, typename B>
    [[nodiscard]] constexpr Ordering compare_three_way(A a, B b)
    {
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be arithmetic.");

        constexpr bool a_is_int = std::is_integral_v<A>;
        constexpr bool b_is_int = std::is_integral_v<B>;

        if constexpr (a_is_int != b_is_int)
        {
            return impl::compare_int_float_three_way(a, b);
        }
        else // a_is_int && b_is_int
        {
            // This looks silly to me.
            if (less(a, b))
                return Ordering::less;
            if (greater(a, b))
                return Ordering::greater;
            if (equal(a, b))
                return Ordering::equal;

            return Ordering::unordered;
        }
    }


    // Returns true if `value` can be represented as `A`.
    template <typename A, Meta::deduce..., typename B>
    [[nodiscard]] constexpr bool representable_as(B value)
    {
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be arithmetic.");
        return equal(value, A(value));
    }

    // Returns true if `value` can not be represented as `A`.
    template <typename A, Meta::deduce..., typename B>
    [[nodiscard]] constexpr bool not_representable_as(B value)
    {
        return !representable_as<A>(value);
    }


    // Attempts to convert `value` to type `A`. Throws if it's not represesntable as `A`.
    // Usage `Robust::safe_cast<A>(value)`.
    template <typename A, Meta::deduce..., typename B>
    [[nodiscard]] constexpr A safe_cast(B value)
    {
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be arithmetic.");
        A result = value;
        if (not_equal(result, value))
            throw std::runtime_error("The value can't be represented by the specified type.");
        return result;
    }

    // Does `dst = src`.
    // If the value of `src` couldn't be represented by `dst` exactly, returns `true`.
    template <Meta::deduce..., typename A, typename B>
    [[nodiscard]] constexpr bool conversion_fails(A src, B &dst)
    {
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be arithmetic.");
        dst = src;
        return not_equal(src, dst);
    }
}
