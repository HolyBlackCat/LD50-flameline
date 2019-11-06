#pragma once

#include <cmath>
#include <limits>
#include <type_traits>

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
        // If a magnitude (aka absolute value) of a floating-point number is less than this value, the number can be fractional.
        template <typename F> inline constexpr F float_frac_threshold = []{
            // Computes `2^(std::numeric_limits<float>::digits-1)` at compile-time.
            static_assert(std::is_floating_point_v<F>);
            F ret = 1;
            F factor = 2;
            unsigned int power = std::numeric_limits<float>::digits-1;
            while (power > 0)
            {
                if (power & 1)
                    ret *= factor;
                factor *= factor;
                power >>= 1;
            }
            return ret;
        }();

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

                // ** Handle special cases.

                if (std::isnan(f))
                    return Ordering::unordered;
                else if (std::isinf(f))
                    return f > 0 ? Ordering::less : Ordering::greater;

                if (std::is_unsigned_v<I> && f < 0)
                    return Ordering::greater;

                // I_magnitude_bits == sizeof(I) * CHAR_BIT - std::is_signed_v<I>
                constexpr int I_magnitude_bits = std::numeric_limits<I>::digits;
                constexpr int F_max_exp = std::numeric_limits<F>::max_exponent;

                // ** Check if `f` is outside of the range `I` can represent.
                if constexpr (F_max_exp > I_magnitude_bits)
                {
                    int exp = 0;
                    F frac = std::frexp(f, &exp);

                    bool float_too_large = exp > I_magnitude_bits + (std::is_signed_v<I> && frac == F(-0.5));
                    if (float_too_large)
                        return f > 0 ? Ordering::less : Ordering::greater;
                }

                // ** Truncate `f` if it's not an integer.

                Ordering result_if_eq = Ordering::equal;
                if (f < float_frac_threshold<F> && f > -float_frac_threshold<F>)
                {
                    F trunc_f = std::trunc(f);
                    if (f != trunc_f)
                    {
                        result_if_eq = f > 0 ? Ordering::less : Ordering::greater;
                        f = trunc_f;
                    }
                }

                // ** Actually compare the numbers.

                I f_as_int = f;

                if (i > f_as_int)
                    return Ordering::greater;
                else if (i < f_as_int)
                    return Ordering::less;
                else
                    return result_if_eq;
            }
        }
    }


    template <typename A, typename B>
    [[nodiscard]] constexpr bool equal(A a, B b)
    {
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be integral or floating-point.");

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
        return !equal(a, b);
    }


    template <typename A, typename B>
    [[nodiscard]] constexpr bool less(A a, B b)
    {
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be integral or floating-point.");

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


    template <typename A, typename B>
    [[nodiscard]] constexpr Ordering compare_three_way(A a, B b)
    {
        static_assert(std::is_arithmetic_v<A> && std::is_arithmetic_v<B>, "Parameters must be integral or floating-point.");

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
    template <typename A, typename B>
    [[nodiscard]] constexpr bool representable_as(B value)
    {
        return equal(value, A(value));
    }
}
