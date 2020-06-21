#pragma once

#include <cmath>
#include <concepts>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include "meta/basic.h"

/* The functions in this file can be used for ROBUST comparisons between integral and/or floating-point types.
 *
 * About reliability:
 * The `float x float` comparisons should be completely reliable, as we simply use the builtin comparison operators.
 * The `int x int` comparisons should also be reliable, as the comparison algorithm is simple.
 * The `int x float` comparisons, on the other hand, rely on a complicated algorithm. Even though they were tested,
 *     it's hard to guarantee complete robustness here. Also they might be slow.
 */

namespace Robust
{
    // A partial ordering.
    // "Partial" means that some values are not comparable (i.e. NaNs, that's what `unordered` is for).
    enum class Ordering {less, equal, greater, unordered};

    // This set of operators allows comparisons between an `Ordering` and 0.
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
        // See following thread for the explanation of the algorithm and for alternative implementations:
        //   https://stackoverflow.com/questions/58734034/how-to-properly-compare-an-integer-and-a-floating-point-value
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
        if constexpr (std::is_same_v<A, B>)
            return false;
        else
            return not_equal(src, dst);
    }


    // Arithmetic operations for integral types with overflow checks and divide-by-zero checks.
    // All three arguments must have the same type; the third one is a reference, that's where the result is stored.
    // Return false on success, and true if the operation fails.
    // If the operation overflows, the returned result should be gracefully wrapped around.
    // If division-by-zero happens, the result is as if the divisor was 1 instead of 0.

    // Concept: an integral type that's not a bool, not cv-qualified.
    // We don't want bools because the overflow-checking builtins don't work on them
    // (actually they do work in Clang, but the GCC manual says they shouldn't, and GCC rejects them),
    // and working around that isn't really worth the effort.
    template <typename T>
    concept integral_non_bool = std::integral<T> && !std::is_same_v<T, bool> && !std::is_const_v<T> && !std::is_volatile_v<T>;

    // Addition.
    template <integral_non_bool C, std::same_as<C> A, std::same_as<C> B>
    constexpr bool addition_fails(A a, B b, C &c)
    {
        return __builtin_add_overflow(a, b, &c);
    }

    // Subtraction.
    template <integral_non_bool C, std::same_as<C> A, std::same_as<C> B>
    constexpr bool subtraction_fails(A a, B b, C &c)
    {
        return __builtin_sub_overflow(a, b, &c);
    }

    // Multiplication.
    template <integral_non_bool C, std::same_as<C> A, std::same_as<C> B>
    constexpr bool multiplication_fails(A a, B b, C &c)
    {
        return __builtin_mul_overflow(a, b, &c);
    }

    // Division.
    template <integral_non_bool C, std::same_as<C> A, std::same_as<C> B>
    constexpr bool division_fails(A a, B b, C &c)
    {
        if (b == B{})
        {
            // Division by zero.
            c = a;
            return true;
        }
        if (std::is_signed_v<A> && a == std::numeric_limits<A>::min && b == B(-1))
        {
            // Division by -1 when `A` is signed and `a` is is the minimal representable value.
            c = a; // Wrap around doesn't change this value.
            return true;
        }

        c = a / b;
        return false;
    }

    // Remainder.
    template <integral_non_bool C, std::same_as<C> A, std::same_as<C> B>
    constexpr bool remainder_fails(A a, B b, C &c)
    {
        if (b == B{})
        {
            c = C{};
            return true;
        }
        c = a % b;
        return false;
    }


    // Integer wrapper, with safe overloaded operators.
    //
    // Example minimal usage:
    //   int a = 10, b = 20, result;
    //   if (Robust::value(a) + Robust::value(b) >>= result)
    //       throw "overflow";
    //   std::cout << result << '\n';
    //
    // The overloaded operators only accept operands of the same type. Use `.cast_to<T>()` to safely cast them if necessary.
    // Alternatively, call `.weakly_typed()` on one of the operands to convert it automatically.
    //
    // If the type of the rhs of `>>=` differs from the stored type, a `cast_to` is performed automatically.
    // Instead of using `>>=` to access the result, you can compare it for [in]equality with an other `Robust::value` (invalid values always compare inequal),
    // There are other ways of accessing the result, see the definition of `class value`.
    //
    // For literals, instead of writing `Robust::value(42).weakly_typed()` you can use `Robust::constant<42>` as a shorthand.

    namespace impl
    {
        class [[nodiscard("Check the return value to see if the result is valid.")]]
            boolean
        {
            bool value = false;

          public:
            constexpr boolean(bool value) : value(value) {}

            boolean(const boolean &) = delete;
            boolean &operator=(const boolean &) = delete;

            [[nodiscard]] explicit constexpr operator bool() const
            {
                return value;
            }
        };
    }

    template <integral_non_bool T>
    class [[nodiscard]] value
    {
        template <integral_non_bool> friend class value;
        T val{};
        bool invalid = false;

        [[nodiscard]] constexpr bool is_zero() const
        {
            return !invalid && val == T{};
        }

        class [[nodiscard]] weakly_typed_wrapper
        {
            value obj;

          public:
            constexpr weakly_typed_wrapper(value obj) : obj(obj) {}

            template <typename U>
            constexpr operator value<U>() const
            {
                return obj.cast_to<U>();
            }
        };

      public:
        constexpr value() {}

        explicit constexpr value(T val) : val(val) {}
        explicit constexpr value(auto) = delete; // Prevent implicit conversions when constructing.

        [[nodiscard]] constexpr bool is_invalid() const
        {
            return invalid;
        }
        [[nodiscard]] constexpr T get_value_or_throw() const
        {
            if (invalid)
                throw std::runtime_error("An invalid `Robust::value`.");
            return val;
        }
        [[nodiscard]] constexpr T get_value_even_if_invalid() const
        {
            return val;
        }
        [[nodiscard]] constexpr T get_value_or(T alternative) const
        {
            if (invalid)
                return alternative;
            return val;
        }
        [[nodiscard]] constexpr T get_value_or(auto) const = delete; // Prevent implicit conversions when constructing.

        // Writes the value of `val` into the `result`.
        // Returns true if the value is invalid, or would be invalid after the cast to `U`.
        template <integral_non_bool U>
        constexpr impl::boolean operator>>=(U &result)
        {
            if constexpr (std::is_same_v<T, U>)
            {
                result = val;
                return invalid;
            }
            else
            {
                return cast_to<U>() >>= result;
            }
        }

        // A safe cast.
        template <integral_non_bool U>
        constexpr value<U> cast_to() const
        {
            value<U> ret;
            ret.invalid = conversion_fails(val, ret.val) || invalid;
            return ret;
        }

        // Returns the same object, wrapped in a class that makes it implicitly convertible to `value<U>` for any `U`.
        constexpr weakly_typed_wrapper weakly_typed() const
        {
            return *this;
        }

        // Arithmetic operations.
        // For extra clarity, the operands are required to have the same type.
        // Note that those are not defined as regular member functions to allow lhs to be of a different type, convertible to `value`.
        friend constexpr value<T> operator+(value a, value b)
        {
            value ret;
            ret.invalid = addition_fails(a.val, b.val, ret.val) || a.invalid || b.invalid;
            return ret;
        }
        friend constexpr value<T> operator-(value a, value b)
        {
            value ret;
            ret.invalid = subtraction_fails(a.val, b.val, ret.val) || a.invalid || b.invalid;
            return ret;
        }
        friend constexpr value<T> operator*(value a, value b)
        {
            // Note that multiplying by a valid 0 ignores the validity of the other operand.
            value ret;
            ret.invalid = multiplication_fails(a.val, b.val, ret.val) || (a.invalid && !b.is_zero()) || (b.invalid && !a.is_zero());
            return ret;
        }
        friend constexpr value<T> operator/(value a, value b)
        {
            // Note that we can't ignore the validity of `b` even if `a` is zero, because `0 / 0` is undefined.
            value ret;
            ret.invalid = division_fails(a.val, b.val, ret.val) || a.invalid || b.invalid;
            return ret;
        }
        friend constexpr value<T> operator%(value a, value b)
        {
            // Note that we can't ignore the validity of `b` even if `a` is zero, because `0 % 0` is undefined.
            // We could in theory ignore the validity of `a` if `abs(b) == 1`, but this is a rather rare case.
            value ret;
            ret.invalid = remainder_fails(a.val, b.val, ret.val) || a.invalid || b.invalid;
            return ret;
        }


        // Equality comparison.
        // In C++20, `a != b` can get automatically rewritten as `!(a == b)`, so we don't overload `!=` manually.
        // We don't provide relational comparisons (less-than, etc) because invalid values would have to
        // be treated as incomparable (like nans when comparing floats), which would make things rather obscure.
        //
        // The non-template friend overload is necessary to support comparison with `Robust::constant`.
        // The template overload is needed for heterogenous comparison. I considered making it friend
        // for consistency, but then it would have to be defined outside of the class.
        [[nodiscard]] friend constexpr bool operator==(value a, value b)
        {
            return !a.invalid && !b.invalid && a.val == b.val;
        }
        template <integral_non_bool U>
        [[nodiscard]] constexpr bool operator==(value<U> other) const
        {
            return !invalid && !other.invalid && equal(val, other.val);
        }
    };

    namespace impl
    {
        template <integral_non_bool auto V>
        class [[nodiscard]] constant
        {
          public:
            constexpr constant() {}

            template <typename T>
            constexpr operator value<T>() const
            {
                // Not sure why it doesn't work without specifying the template parameter. A Clang bug?
                return value<decltype(V)>(V).template cast_to<T>();
            }
        };
    }

    template <integral_non_bool auto V>
    inline constexpr impl::constant<V> constant;
}
