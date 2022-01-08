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
    auto random_generator = Random::MakeGeneratorFromRandomDevice();
    Random::DefaultInterfaces ra(random_generator);

Usage:

* Random int:
    A <= ra.i       <= B
         ra.i       <= B // Implies `0 <= ra.i`.
         ra.i.abs() <= B // Implies `-B <= ra.i`.
  Both `<` and `<=` are supported.
  Types of bounds don't affect the resulting type.
  Bounds can even have different floating-point-ness compared to the target type.

* Random float:
  Same, but using `ra.f`.

* Random vector:
    A <= ra.{i,f}vec{2,3,4} <= B
  For a vector, both scalar and vector bounds are supported.

See `Random::Misc` for various helpers.
*/

namespace Random
{
    using DefaultGenerator = std::mt19937;

    // Constructs an `std::seed_seq` of size `count` by repeatedly calling `func()`, which must return `uint32_t`.
    // Then uses that sequence to create a generator of type `Generator`.
    template <typename Generator = DefaultGenerator, typename F>
    [[nodiscard]] Generator MakeGenerator(std::size_t count, F &&func)
    {
        static_assert(std::is_same_v<decltype(func()), std::uint32_t>, "The functor must return `uint32_t`.");

        std::vector<std::uint32_t> vec;
        vec.reserve(count);
        while (count-- > 0)
            vec.push_back(func());

        std::seed_seq seq(vec.begin(), vec.end());
        return Generator(seq);
    }

    // Same as `MakeGenerator`, but always uses `std::random_device` for the seed.
    template <typename Generator = DefaultGenerator>
    [[nodiscard]] Generator MakeGeneratorFromRandomDevice(std::size_t count = 4)
    {
        std::random_device r;
        return (MakeGenerator<Generator>)(count, [&]() -> std::uint32_t {return r();});
    }


    namespace impl
    {
        template <typename T> struct UniformDistribution {};
        template <std::signed_integral T> struct UniformDistribution<T> {using type = std::uniform_int_distribution<T>;};
        template <std::floating_point T> struct UniformDistribution<T> {using type = std::uniform_real_distribution<T>;};
    }

    // Signed integral or floating-point type.
    template <typename T>
    concept SupportedScalar = requires{typename impl::UniformDistribution<T>::type;};

    // A single `SupportedScalar` or a vector of them.
    template <typename T>
    concept SupportedScalarOrVec = SupportedScalar<Math::vec_base_t<T>>;

    // Any container.
    template <typename T>
    concept Container = requires(const T &t)
    {
        std::begin(t);
        std::end(t);
        std::size(t);
    };

    // A random-access container.
    template <typename T>
    concept RandomAccessContainer = Container<T> && requires(const T &t)
    {
        requires std::is_same_v<typename std::iterator_traits<decltype(std::begin(t))>::iterator_category, std::random_access_iterator_tag>;
    };

    // A random-access container of pairs (or 2-tuple-like classes),
    // with the first element having an arithmetic type (to be used as weight).
    template <typename T>
    concept WeightedRandomAccessContainer = RandomAccessContainer<T> && requires(const T &t, std::remove_cvref_t<decltype(*std::begin(t))> e)
    {
        requires std::tuple_size<decltype(e)>::value == 2; // Note, not using SFINAE-unfriendly `std::tuple_size_v`.
        requires std::is_arithmetic_v<std::remove_cvref_t<decltype(std::get<0>(e))>>;
    };


    namespace impl
    {
        // Converts the parameter to a number that can be properly printed.
        // I.e. converts `char`s to `int`s, and leaves everything else unchanged.
        template <SupportedScalar T> T ToPrintableNumber(T value) {return value;}
        inline int ToPrintableNumber(char value) {return int(value);}


        // Returns the next or previous representable value.
        // Refuses to increment the largest representable value, and returns it unchanged.
        // If asked to increment infinity in either direction, returns the closest representable value.
        // If given NaN, returns NaN.
        template <bool Decrease, Meta::deduce..., SupportedScalar T>
        [[nodiscard]] T NextRepresentable(T value)
        {
            constexpr auto limit = Decrease ? std::numeric_limits<T>::lowest() : std::numeric_limits<T>::max();
            if constexpr (std::is_floating_point_v<T>)
                return std::nextafter(value, limit);
            else
                return value == limit ? limit : value + (Decrease ? -1 : 1);
        }

        enum BoundType {upper, lower};

        // Converts the `value` to a suitable upper/lower inclusive bound for a scalar type `T`.
        // Should never return an infinity, but can return NaN.
        template <SupportedScalar TargetT, BoundType Bound, Meta::deduce..., SupportedScalar SourceT>
        [[nodiscard]] TargetT MakeInclusiveScalarBound(SourceT source)
        {
            if constexpr (std::is_floating_point_v<TargetT>)
            {
                // This can lead to loss of precision or overflow if both `TargetT` and `SourceT` are floating-point (and `SourceT` is a larger type).
                // We check for overflow below, but we don't care about the precision loss.
                // If `SourceT` is integral, this should never overflow (at least if you don't use non-standard large integers or tiny floats),
                // but it can still cause a loss of precision.
                TargetT ret(source);

                // Replace infinity with the closest representable value.
                if constexpr (std::is_floating_point_v<SourceT>)
                {
                    if (std::isinf(ret))
                        ret = std::nextafter(ret, 0.f);
                }

                return ret;
            }
            else if constexpr (std::is_integral_v<SourceT> && std::is_integral_v<TargetT>)
            {
                // Clamp to the bounds of the target type.
                if (Robust::compare(source) <= std::numeric_limits<TargetT>::lowest())
                    return std::numeric_limits<TargetT>::lowest();
                if (Robust::compare(source) >= std::numeric_limits<TargetT>::max())
                    return std::numeric_limits<TargetT>::max();

                return TargetT(source);
            }
            else if constexpr (std::integral<TargetT>)
            {
                constexpr SourceT min_result = std::numeric_limits<TargetT>::lowest();
                // Check for overflow in `min_result`. It shouldn't happen, unless you use non-standard types.
                static_assert(min_result != min_result * 2, "The integral type is too large and the floating-point is too small. This shouldn't happen with standard types.");

                if (source <= min_result)
                    return std::numeric_limits<TargetT>::lowest();
                if (source + min_result >= -1) // We use this trick, because the upper limit of `TargetT` is not a power of two, so it might not be representable in `SourceT`.
                    return std::numeric_limits<TargetT>::max();

                // Round `source` in the proper direction.
                if constexpr (Bound == upper)
                    source = std::floor(source);
                else
                    source = std::ceil(source);

                // Now the cast should be exact.
                return TargetT(source);
            }
            else
            {
                static_assert(Meta::value<false, TargetT, SourceT>, "This shouldn't happen.");
            }
        }

        // Same as `MakeInclusiveScalarBound`, but supports vector types and is optionally exclusive.
        template <SupportedScalarOrVec TargetT, BoundType Bound, Meta::deduce..., SupportedScalarOrVec SourceT>
        requires (Math::vector<SourceT> <=/*implies*/ Math::vector<TargetT>)
        [[nodiscard]] TargetT MakeBound(SourceT source, bool is_exclusive)
        {
            if constexpr (Math::vector<SourceT>)
            {
                // If both `TargetT` and `SourceT` are vectors.
                return Math::apply_elementwise([&is_exclusive](auto source)
                {
                    return (MakeBound<Math::vec_base_t<TargetT>, Bound>)(source, is_exclusive);
                }, source);
            }
            else if constexpr (Math::vector<TargetT>)
            {
                // If `TargetT` is a vector, but `SourceT` is not.
                return TargetT((MakeBound<Math::vec_base_t<TargetT>, Bound>)(source, is_exclusive));
            }
            else
            {
                // If neither `TargetT` nor `SourceT` are vectors.

                // This seems to work.
                if (is_exclusive && std::is_floating_point_v<SourceT>)
                    source = (NextRepresentable<Bound == upper>)(source);
                TargetT ret = (MakeInclusiveScalarBound<TargetT, Bound>)(source);
                if (is_exclusive && !std::is_floating_point_v<SourceT>)
                    ret = (NextRepresentable<Bound == upper>)(ret);

                return ret;
            }
        }

        // Generates a random number. Both bounds are inclusive.
        template <SupportedScalarOrVec T, typename Distribution, typename Generator>
        [[nodiscard]] static T GenerateNumber(Generator &gen, Distribution &dist, T a, T b)
        {
            return Math::apply_elementwise([&](auto min_value, auto max_value)
            {
                // Even though the upper bound of `std::uniform_real_distribution` is said to be exclusive,
                //   cppreference claims (https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution)
                //   that most implementations are bugged and treat it as an inclusive bound.
                // IMO, theoretically undershooting the upper bound is better than overshooting it,
                //   so we consider both bounds to be inclusive for all types.

                if constexpr (std::floating_point<Math::vec_base_t<T>>)
                {
                    // It's still UB to pass two same values to `std::uniform_real_distribution`, so we check for it.
                    if (min_value == max_value)
                        return min_value;

                    // If one of the bounds is nan, return the other one.
                    // If both are nan, return zero.
                    // This also checks for infinity, just in case, even though `MakeBound` should never return it.
                    if (!std::isfinite(min_value))
                    {
                        ASSERT(false, "Non-finite random number range.");
                        return std::isfinite(max_value) ? max_value : 0;
                    }
                    if (!std::isfinite(max_value))
                    {
                        ASSERT(false, "Non-finite random number range.");
                        return min_value;
                    }
                }

                // Make sure the range is valid.
                if (min_value > max_value)
                {
                    ASSERT(false, FMT("Invalid random number range: min={} is greater than max={}.", ToPrintableNumber(min_value), ToPrintableNumber(max_value)));
                    std::swap(min_value, max_value);
                }

                dist.param(typename Distribution::param_type(min_value, max_value));
                return dist(gen);
            }, a, b);
        }
    }

    // `P...` must be unique.
    template <typename Generator, SupportedScalarOrVec T>
    class Interface
    {
      public:
        using type = T;

      private:
        Generator *gen = nullptr;

        using dist_t = typename impl::UniformDistribution<Math::vec_base_t<T>>::type;
        dist_t dist;

        class HalfRange
        {
            Interface *in = nullptr;
            T min{};

          public:
            HalfRange(Interface &in, T min) : in(&in), min(min) {}

            HalfRange(const HalfRange &) = delete;
            HalfRange &operator=(const HalfRange &) = delete;

            [[nodiscard]] T operator< (SupportedScalarOrVec auto max) && {return impl::GenerateNumber(*in->gen, in->dist, min, impl::MakeBound<T, impl::BoundType::upper>(max, true ));}
            [[nodiscard]] T operator<=(SupportedScalarOrVec auto max) && {return impl::GenerateNumber(*in->gen, in->dist, min, impl::MakeBound<T, impl::BoundType::upper>(max, false));}
        };

        class SymmetricRange
        {
            Interface *in = nullptr;

          public:
            SymmetricRange(Interface &in) : in(&in) {}

            SymmetricRange(const SymmetricRange &) = delete;
            SymmetricRange &operator=(const SymmetricRange &) = delete;

            [[nodiscard]] T operator< (SupportedScalarOrVec auto max) && {return impl::GenerateNumber(*in->gen, in->dist, impl::MakeBound<T, impl::BoundType::lower>(-max, true ), impl::MakeBound<T, impl::BoundType::upper>(max, true ));}
            [[nodiscard]] T operator<=(SupportedScalarOrVec auto max) && {return impl::GenerateNumber(*in->gen, in->dist, impl::MakeBound<T, impl::BoundType::lower>(-max, false), impl::MakeBound<T, impl::BoundType::upper>(max, false));}
        };

      public:
        Interface(Generator &gen) : gen(&gen) {}

        Interface(const Interface &) = delete;
        Interface &operator=(const Interface &) = delete;

        [[nodiscard]]       Generator &GetGenerator()       {return *gen;}
        [[nodiscard]] const Generator &GetGenerator() const {return *gen;}

        [[nodiscard]] T operator< (SupportedScalarOrVec auto max) {return impl::GenerateNumber(*gen, dist, T{}, impl::MakeBound<T, impl::BoundType::upper>(max, true ));}
        [[nodiscard]] T operator<=(SupportedScalarOrVec auto max) {return impl::GenerateNumber(*gen, dist, T{}, impl::MakeBound<T, impl::BoundType::upper>(max, false));}

        [[nodiscard]] friend HalfRange operator< (SupportedScalarOrVec auto min, Interface &in) {return HalfRange(in, impl::MakeBound<T, impl::BoundType::lower>(min, true ));}
        [[nodiscard]] friend HalfRange operator<=(SupportedScalarOrVec auto min, Interface &in) {return HalfRange(in, impl::MakeBound<T, impl::BoundType::lower>(min, false));}

        [[nodiscard]] SymmetricRange abs() {return {*this};}
    };

    // Helper for generating various random things.
    template <typename Generator, SupportedScalar Real>
    requires std::floating_point<Real>
    class Misc
    {
        Interface<Generator, Real> float_helper;
        Interface<Generator, std::ptrdiff_t> index_helper;
        std::discrete_distribution<std::ptrdiff_t> discrete_dist;

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
        [[nodiscard]] Real angle()
        {
            static const float min = -pi<Real>(), max = std::nextafter(pi<Real>(), 0.f);
            return min <= float_helper <= max;
        }

        // Returns a random index, `0 <= x < max`.
        [[nodiscard]] std::ptrdiff_t index(std::ptrdiff_t max)
        {
            return index_helper < max;
        }

        // Returns a random index, `0 <= x < probs.size()`.
        // `probs` must store arithmetic types, which are used as relative probabilities for the corresponding indices.
        template <typename T> requires std::is_arithmetic_v<T>
        [[nodiscard]] std::ptrdiff_t index_weighted(std::initializer_list<T> probs)
        {
            return index_weighted<decltype(probs)>(probs);
        }

        // Returns a random index, `0 <= x < probs.size()`.
        // `probs` must store arithmetic types, which are used as relative probabilities for the corresponding indices.
        template <Container C>
        [[nodiscard]] std::ptrdiff_t index_weighted(const C &probs)
        requires std::is_arithmetic_v<std::remove_cvref_t<decltype(*std::begin(probs))>>
        {
            discrete_dist.param({std::begin(probs), std::end(probs)});
            return discrete_dist(GetGenerator());
        }

        // Returns a random element from a list. Throws if the list is empty.
        template <typename T>
        [[nodiscard]] const T &choose(std::initializer_list<T> list)
        {
            return choose<decltype(list)>(list);
        }

        // Returns a random element from a container. Throws if the container is empty.
        // The container needs to have random-access iterators. Plain arrays are supported.
        template <RandomAccessContainer C>
        [[nodiscard]] auto choose(const C &container) -> decltype(*std::begin(container))
        {
            return *(std::begin(container) + index(std::size(container)));
        }

        // Given a list of pairs, returns the second element of one of them.
        // The first elements are used as relative probabilities.
        // Example usage:
        //     choose_weighted({std::pair{1,"a"},std::pair{2,"b"},std::pair{3,"c"}});
        // It's enough to use `std::pair` on the first element only, but then it dictates the deduction for all other elements.
        template <typename A, typename B>
        [[nodiscard]] const B &choose_weighted(std::initializer_list<std::pair<A, B>> list)
        requires WeightedRandomAccessContainer<decltype(list)>
        {
            return choose_weighted<decltype(list)>(list);
        }

        // Given a container of pairs (2-tuple-like types), returns the second element of one of them.
        // The first elements are used as relative probabilities.
        // The container needs to have random-access iterators. Plain arrays are supported.
        template <WeightedRandomAccessContainer C>
        [[nodiscard]] auto choose_weighted(const C &container) -> decltype(std::get<1>(*std::begin(container)))
        {
            std::size_t i = 0;
            discrete_dist.param({std::size(container), 0.0, 0.0, [&](double){return std::get<0>(std::begin(container)[i++]);}});
            return std::get<1>(*(std::begin(container) + discrete_dist(GetGenerator())));
        }
    };

    template <typename Generator>
    class DefaultInterfaces : public Misc<Generator, float>
    {
      public:
        DefaultInterfaces(Generator &gen)
            : DefaultInterfaces::Misc(gen),
            i(gen), ivec2(gen), ivec3(gen), ivec4(gen),
            f(gen), fvec2(gen), fvec3(gen), fvec4(gen)
        {}

        Interface<Generator, int> i;
        Interface<Generator, ivec2> ivec2;
        Interface<Generator, ivec3> ivec3;
        Interface<Generator, ivec4> ivec4;

        Interface<Generator, float> f;
        Interface<Generator, fvec2> fvec2;
        Interface<Generator, fvec3> fvec3;
        Interface<Generator, fvec4> fvec4;
    };
}
