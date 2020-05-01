#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <ostream>
#include <random>
#include <type_traits>
#include <utility>

#include "program/platform.h"
#include "utils/robust_math.h"

template <typename DefaultInt = int, typename DefaultReal = float>
class Random
{
    static_assert(std::is_integral_v<DefaultInt> && !std::is_same_v<DefaultInt, bool>, "DefaultInt must be integral.");
    static_assert(std::is_floating_point_v<DefaultReal>, "DefaultReal must be floating-point.");

    // Generates numbers distributed in a continuous range.
    template <typename T, bool HasMin = 0, bool HasMax = 0> class Range
    {
        friend class Random;

        static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "The type has to be integral or floating point.");
        static constexpr bool is_integral = std::is_integral_v<T>;
        static constexpr bool is_floating_point = !is_integral;

        // Representable range for the result type.
        static constexpr T min_limit = std::numeric_limits<T>::lowest(); // Sic!
        static constexpr T max_limit = std::numeric_limits<T>::max();

        // Default distribution range.
        static constexpr T default_min = 0;
        static constexpr T default_max = max_limit;

        // Get next/previous representable number.
        // For floating-point numbers those never increment the number into infinity.
        // For integers there is no overflow check, but it shouldn't be necessary.
        template <typename U> static U next(U value)
        {
            if constexpr (std::is_floating_point_v<U>)
                return std::nextafter(value, max_limit);
            else
                return value + 1;
        }
        template <typename U> static U prev(U value)
        {
            if constexpr (std::is_floating_point_v<U>)
                return std::nextafter(value, min_limit);
            else
                return value - 1;
        }

        // Make distribution object from range. The range is inclusive: [min,max].
        static auto make_distribution(T min, T max)
        {
            if (min > max)
                std::swap(min, max);

            if constexpr (is_integral)
                return std::uniform_int_distribution<T>(min, max);
            else
                return std::uniform_real_distribution<T>(min, next(max)); // Sic!
        }

        // Prepare min/max bound.
        // If T is integer, ceil/floor is applied.
        // The value is clamped to the representable range.
        template <typename U> static T make_min_bound(U value)
        {
            if constexpr (is_integral)
            {
                auto val = std::llround(std::ceil(value));
                if (Robust::less(val, min_limit)) return min_limit;
                if (Robust::greater(val, max_limit)) return max_limit;
                return val;
            }
            else
            {
                T x = value;
                return x >= min_limit && x <= max_limit ? x : x > 0 ? max_limit : min_limit; // This should ensure a proper handling of NaNs and inifinte values.
            }
        }
        template <typename U> static T make_max_bound(U value)
        {
            if constexpr (is_integral)
            {
                auto val = std::llround(std::floor(value));
                if (Robust::less(val, min_limit)) return min_limit;
                if (Robust::greater(val, max_limit)) return max_limit;
                return val;
            }
            else
            {
                T x = value;
                return x >= min_limit && x <= max_limit ? x : x < 0 ? min_limit : max_limit; // This should ensure a proper handling of NaNs and inifinte values.
            }
        }

        Random *source = 0;

        using distribution_t = decltype(make_distribution(0,0));
        distribution_t distribution = make_distribution(0, 0);

      public:
        Range() {}
        Range(Random *source) : Range(source, default_min, default_max) {} // Note that the default lower bound is zero.
        Range(Random *source, T min, T max) : source(source), distribution(make_distribution(min, max)) {}

        // Get the range (inclusive).
        T min() const
        {
            return distribution.a();
        }
        T max() const
        {
            // We use this logic instead of `.max()`, because it seems to return the same value as `.b()` even for floating point types.
            if constexpr (is_integral)
                return distribution.b();
            else
                return prev(distribution.b()); // Sic!
        }

        // Use <,>,<=,>= to change the range.
        // Using different types should be fine, even floating-point ones with integral ranges.
        // The resulting number will satisfy all provided inequalities, with more priority given to the last applied ones.
        // If no lower bound is set, it defaults to 0.
        template <typename U> friend auto operator<=(Range expr, U value)
        {
            T x = make_max_bound(value);
            return Range<T, HasMin, 1>(expr.source, std::min(expr.min(), x), std::min(HasMax ? expr.max() : max_limit, x));
        }
        template <typename U> friend auto operator>=(Range expr, U value)
        {
            T x = make_min_bound(value);
            return Range<T, 1, HasMax>(expr.source, std::max(HasMin ? expr.min() : min_limit, x), std::max(expr.max(), x));
        }
        template <typename U> friend auto operator<(Range expr, U value) {return expr <= prev(value);}
        template <typename U> friend auto operator>(Range expr, U value) {return expr >= next(value);}
        template <typename U> friend auto operator<=(U value, Range expr) {return expr >= value;}
        template <typename U> friend auto operator>=(U value, Range expr) {return expr <= value;}
        template <typename U> friend auto operator<(U value, Range expr) {return expr > value;}
        template <typename U> friend auto operator>(U value, Range expr) {return expr < value;}

        // This generates a new value based on current constraints.
        operator T()
        {
            static_assert(is_integral || HasMax, "An explicit upper bound is required for floating-point types.");
            T ret = distribution(source->generator());
            if constexpr (is_floating_point) // I heard that floating point distributions on some implementations sometimes go slightly out of range.
            {
                if (auto x = min(); ret < x)
                    return x;
                if (auto x = max(); ret > x)
                    return x;
            }
            return ret;
        }

        // A printing operator.
        template <typename A, typename B> friend std::basic_ostream<A,B> &operator<<(std::basic_ostream<A,B> &stream, Range expr)
        {
            stream << T(expr);
            return stream;
        }
    };

    // Wraps a number-generating function.
    template <typename F> class Function
    {
        F func;
      public:
        Function(F &&func) : func(std::move(func)) {}

        operator decltype(std::declval<F>()())()
        {
            return func();
        }
    };

    // Clang doesn't want to do CTAD without this guide, even though it seems redundant. It seems like a Clang bug, but I'm not sure.
    // GCC, on the other hand, works without the guide and refuses to compile it (because it's not at namespace scope), which is certainly a GCC bug.
    PLATFORM_IF(gcc)( template <typename F> Function(F &&) -> Function<F>; )

  public:
    using generator_t = std::mt19937;
    using result_t = generator_t::result_type;
    using seed_t = result_t;

  private:
    generator_t gen;

  public:
    Random() {}

    Random(seed_t seed) : gen(seed) {}

    void set_seed(seed_t seed)
    {
        gen.seed(seed);
    }

    generator_t &generator()
    {
        return gen;
    }

    // All of the functions below return not a number, but an object that generates numbers lazily when cast to T.

    template <typename T> Range<T> type()
    {
        return Range<T>(this);
    }
    template <typename T = DefaultInt> Range<T> integer()
    {
        static_assert(std::is_integral_v<T>, "The type must be integral.");
        return type<T>();
    }
    template <typename T = DefaultReal> Range<T> real()
    {
        static_assert(std::is_floating_point_v<T>, "The type must be floating point.");
        return type<T>();
    }

    auto boolean()
    {
        return Function([this]() -> bool
        {
            return gen() <= generator_t::max() / 2; // Sic! We use `<=` here on purpose.
        });
    }

    auto sign()
    {
        return Function([this]() -> DefaultInt
        {
            return boolean() * 2 - 1;
        });
    }

    template <typename T = DefaultReal> auto angle()
    {
        constexpr long double pi = 3.14159265358979323846l;
        return Function([this]() -> T
        {
            return -pi < real<T>() <= pi;
        });
    }
};
