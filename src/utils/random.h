#pragma once

#include <cmath>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <utility>

#include "meta/basic.h"
#include "program/errors.h"
#include "utils/mat.h"
#include "utils/robust_math.h"

/* Convenient random number generation helpers.
Suggested minimal setup:
    auto random_generator = Random::RandomDeviceSeedSeq().MakeRng<Random::DefaultGenerator>();
    Random::Scalar<int> irand(random_generator);
    Random::Scalar<float> frand(random_generator);
    Random::Misc<float> miscrand(random_generator);

Usage:
    A <= {i,f}rand <= B  // Types of bounds don't affect the resulting type.
    A <  {i,f}rand <  B  // Bounds can even have different floating-point-ness compared to the target type.
    {i,f}rand <= A       // The default lower bound is 0, inclusive.
    {i,f}rand <  A       // ^
    {i,f}rand.abs() <= A // The lower bound is same as the upper bound negated.
    {i,f}rand.abs() <  A // ^
    miscrand.boolean()   // true, false
    miscrand.sign()      // 1, -1
    miscrand.angle()     // -pi <= x < pi (the type is controlled by the template parameter of `Random::Misc`)
    miscrand.index(N)    // 0 <= x < N (the type is always `std::ptrdiff_t`)
    miscrand.choose({"foo", "bar"})
    miscrand.choose(some_container) // The container must have random-access iterators. Plain arrays are supported.
 */

namespace Random
{
    using DefaultGenerator = std::mt19937;

    // Invokes a function repeatedly to seed a random number generator.
    // Partially conforms to the `SeedSequence` named requirement, just enough to work in practice.
    // Unlike `std::sed_seq`, it doesn't try to scramble the seeds, trusting the user with the quality of the randomness.
    template <typename F>
    class FuncSeedSeq
    {
        static_assert(std::is_same_v<decltype(std::declval<F &>()()), std::uint32_t>, "The function must return `std::uint32_t`.");

        mutable F func;

      public:
        // The standard requires this, and things don't work in practice without it.
        using result_type = std::uint32_t;

        // Constructs `func` using the provided parameters.
        FuncSeedSeq(auto &&... params) : func(decltype(params)(params)...) {}

        // Calls the `func` repeatedly to fill the target range.
        // The standard requires this.
        template <typename T>
        void generate(T target_begin, T target_end) const
        {
            while (target_begin != target_end)
                *target_begin++ = func();
        }

        // Constructs a random number generator, e.g. `std::mt19937`.
        template <typename T>
        [[nodiscard]] T MakeRng() const
        {
            return T(*this);
        }
    };

    namespace impl
    {
        struct RandomDeviceFunctor
        {
            std::random_device r;

            RandomDeviceFunctor() {}

            // The `token` has implementation-defined meaning, see `https://en.cppreference.com/w/cpp/numeric/random/random_device/random_device`.
            RandomDeviceFunctor(const std::string &token) : r(token) {}

            [[nodiscard]] auto operator()()
            {
                return r();
            }
        };
    }

    // A specialization of `FuncSeedSeq` that uses `std::random_device`.
    using RandomDeviceSeedSeq = FuncSeedSeq<impl::RandomDeviceFunctor>;


    namespace impl
    {
        template <typename T> struct UniformDistribution {};
        template <std::signed_integral T> struct UniformDistribution<T> {using type = std::uniform_int_distribution<T>;};
        template <std::floating_point T> struct UniformDistribution<T> {using type = std::uniform_real_distribution<T>;};
    }

    // Signed integral or floating-point type.
    template <typename T>
    concept SupportedScalar = requires{typename impl::UniformDistribution<T>::type;};

    namespace impl
    {
        // Returns the next or previous representable value.
        // If there is no such (finite) value, returns the parameter unchanged.
        template <bool Decrease, Meta::deduce..., SupportedScalar T>
        [[nodiscard]] T NextRepresentable(T value)
        {
            constexpr auto limit = Decrease ? std::numeric_limits<T>::lowest() : std::numeric_limits<T>::max();
            if constexpr (std::floating_point<T>)
                return std::nextafter(value, limit);
            else
                return value == limit ? limit : value + (Decrease ? -1 : 1);
        }

        enum BoundType {upper, lower};

        // Converts a `value` to a suitable upper/lower bound of type `T`.
        template <SupportedScalar T, BoundType Bound, Meta::deduce..., SupportedScalar U>
        [[nodiscard]] T MakeBound(U value)
        {
            if constexpr (std::integral<T> && std::floating_point<U>)
            {
                if constexpr (Bound == upper)
                    value = std::floor(value);
                else
                    value = std::ceil(value);

            }
            if constexpr (std::integral<T>)
            {
                T ret;
                if (Robust::conversion_fails(value, ret))
                    return value < 0 ? std::numeric_limits<T>::lowest() : std::numeric_limits<T>::max();

                return ret;
            }
            else
            {
                // This can lead to loss of precision or overflow if both `T` and `U` are floating-point (and `U` is a larger type), but we don't really care.
                // If `U` is integral, this should never overflow (at least if you don't use non-standard large integers or tiny floats).
                return value;
            }
        }

        // Same, but the bound as exclusive.
        template <SupportedScalar T, BoundType Bound, Meta::deduce..., SupportedScalar U>
        [[nodiscard]] T MakeExclusiveBound(U value)
        {
            // This seems to work.
            if constexpr (std::floating_point<U>)
                value = NextRepresentable<Bound == upper>(value);
            T ret = MakeBound<T, Bound>(value);
            if constexpr (!std::floating_point<U>)
                ret = NextRepresentable<Bound == upper>(ret);
            return ret;
        }
    }

    // Generates a single random scalar. Not thread-safe.
    template <SupportedScalar T, typename Generator = DefaultGenerator>
    class Scalar
    {
        using Distribution = typename impl::UniformDistribution<T>::type;
        Distribution dist;
        Generator *gen = nullptr;

        // Generates a random number. Both bounds are inclusive.
        [[nodiscard]] static T GenerateNumber(Distribution &dist, Generator &gen, T min_value, T max_value)
        {
            // Even though the upper bound of `std::uniform_real_distribution` is said to be exclusive,
            //   cppreference claims (https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution)
            //   that most implementations are bugged and treat it as an inclusive bound.
            // Thus we can consider both bounds to be inclusive in all cases.

            if constexpr (std::floating_point<T>)
            {
                // It's still UB to pass two same values to `std::uniform_real_distribution`, so we check for it.
                if (min_value == max_value)
                    return min_value;

                // If one of the bounds is non-finite, return the other one.
                // If both are non-finite, return zero.
                if (!std::isfinite(min_value))
                {
                    ASSERT(false, "Non-finite `Random::Scalar` range.");
                    return std::isfinite(max_value) ? max_value : 0;
                }
                if (!std::isfinite(max_value))
                {
                    ASSERT(false, "Non-finite `Random::Scalar` range.");
                    return min_value;
                }
            }

            if (min_value > max_value)
            {
                ASSERT(false, "Invalid `Random::Scalar` range.");
                return min_value;
            }

            dist.param(typename Distribution::param_type(min_value, max_value));
            return dist(gen);
        }

        template <int D>
        [[nodiscard]] static vec<D,T> GenerateNumber(Distribution &dist, Generator &gen, const vec<D,T> &min_value, const vec<D,T> &max_value)
        {
            return Math::apply_elementwise([&](T a, T b){return GenerateNumber(dist, gen, a, b);}, min_value, max_value);
        }

        // A helper for generating a random number.
        // Remembers the lower bound, and needs the upper bound to be specified.
        class HalfRange
        {
            Distribution &dist;
            Generator &gen;

            T min_value = 0;

          public:
            HalfRange(Distribution &dist, Generator &gen, T min_value) : dist(dist), gen(gen), min_value(min_value) {}

            HalfRange(const HalfRange &) = delete;
            HalfRange &operator=(const HalfRange &) = delete;

            [[nodiscard]] T operator<=(SupportedScalar auto max_value) && {return GenerateNumber(dist, gen, min_value, impl::MakeBound         <T, impl::upper>(max_value));}
            [[nodiscard]] T operator< (SupportedScalar auto max_value) && {return GenerateNumber(dist, gen, min_value, impl::MakeExclusiveBound<T, impl::upper>(max_value));}
        };

        // A helper for generating a random number.
        // Uses a range symmetric relative to 0, needs the absolute bound to be specified.
        class SymmetricRange
        {
            Distribution &dist;
            Generator &gen;

          public:
            SymmetricRange(Distribution &dist, Generator &gen) : dist(dist), gen(gen) {}

            SymmetricRange(const SymmetricRange &) = delete;
            SymmetricRange &operator=(const SymmetricRange &) = delete;

            [[nodiscard]] T operator<=(SupportedScalar auto max_abs_value) &&
            {
                if (max_abs_value < 0)
                {
                    ASSERT(false, "Negative `Random::Scalar` absolute bound.");
                    return 0;
                }
                return GenerateNumber(dist, gen, impl::MakeBound<T, impl::lower>(-max_abs_value), impl::MakeBound<T, impl::upper>(max_abs_value));
            }

            [[nodiscard]] T operator<(SupportedScalar auto max_abs_value) &&
            {
                if (max_abs_value <=/*sic*/ 0)
                {
                    ASSERT(false, "Negative `Random::Scalar` absolute bound.");
                    return 0;
                }
                return GenerateNumber(dist, gen, impl::MakeExclusiveBound<T, impl::lower>(-max_abs_value), impl::MakeExclusiveBound<T, impl::upper>(max_abs_value));
            }
        };

      public:
        Scalar(Generator &gen) : gen(&gen) {}

        Scalar(const Scalar &) = delete;
        Scalar &operator=(const Scalar &) = delete;

        [[nodiscard]]       Generator &GetGenerator()       {return *gen;}
        [[nodiscard]] const Generator &GetGenerator() const {return *gen;}

        [[nodiscard]] friend HalfRange operator<=(SupportedScalar auto min_value, Scalar &self) {return HalfRange(self.dist, *self.gen, impl::MakeBound         <T, impl::lower>(min_value));}
        [[nodiscard]] friend HalfRange operator< (SupportedScalar auto min_value, Scalar &self) {return HalfRange(self.dist, *self.gen, impl::MakeExclusiveBound<T, impl::lower>(min_value));}

        [[nodiscard]] T operator<=(SupportedScalar auto max_value) {return GenerateNumber(dist, *gen, 0, impl::MakeBound         <T, impl::upper>(max_value));}
        [[nodiscard]] T operator< (SupportedScalar auto max_value) {return GenerateNumber(dist, *gen, 0, impl::MakeExclusiveBound<T, impl::upper>(max_value));}

        [[nodiscard]] SymmetricRange abs()
        {
            return SymmetricRange(dist, *gen);
        }
    };

    // Helper for generating various random things.
    template <SupportedScalar DefaultFloat, typename Generator = DefaultGenerator>
    requires std::floating_point<DefaultFloat>
    class Misc
    {
        Scalar<DefaultFloat> float_helper;
        Scalar<std::ptrdiff_t> index_helper;

      public:
        Misc(Generator &gen) : float_helper(gen), index_helper(gen) {}

        Misc(const Misc &) = delete;
        Misc &operator=(const Misc &) = delete;

        [[nodiscard]]       Generator &GetGenerator()       {return float_helper.GetGenerator();}
        [[nodiscard]] const Generator &GetGenerator() const {return float_helper.GetGenerator();}

        // Returns either true or false.
        [[nodiscard]] bool boolean()
        {
            return bool(GetGenerator()() & 1);
        }

        // Returns either 1 or -1.
        [[nodiscard]] int sign()
        {
            return boolean() ? 1 : -1;
        }

        // Returns a random angle, `-pi <= x < pi`.
        [[nodiscard]] DefaultFloat angle()
        {
            static const float min = -pi<DefaultFloat>(), max = std::nextafter(pi<DefaultFloat>(), 0);
            return min <= float_helper <= max;
        }

        // Returns a random index, `0 <= x < max`.
        [[nodiscard]] std::ptrdiff_t index(std::ptrdiff_t max)
        {
            if (max <= 0)
                Program::Error("Need a positive upper bound for `Random::Misc::index`.");
            return index_helper < max;
        }

        // Returns a random element from a list. Throws if the list is empty.
        template <typename T>
        [[nodiscard]] const T &choose(std::initializer_list<T> list)
        {
            return *(list.begin() + index(list.size()));
        }

        // Returns a random element from a container. Throws if the container is empty.
        // The container needs to have random-access iterators. Plain arrays are supported.
        template <typename C>
        [[nodiscard]] auto choose(const C &container) -> decltype(*std::begin(container))
        requires requires{
            std::size(container);
            requires std::same_as<typename std::iterator_traits<decltype(std::begin(container))>::iterator_category, std::random_access_iterator_tag>;
        }
        {
            return *(std::begin(container) + index(std::size(container)));
        }
    };
}
