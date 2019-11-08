#pragma once

#include <array>
#include <type_traits>
#include <utility>

#include "check.h"

namespace Meta
{
    // Tag dispatch helpers.

    template <typename T> struct tag {using type = T;};
    template <typename...> struct type_list {};
    template <auto...> struct value_list {};


    // Dependent values and types, good for `static_asserts` and SFINAE.

    namespace impl
    {
        template <typename T, typename ...P> struct dependent_type
        {
            using type = T;
        };
    }

    template <auto V, typename, typename...> inline constexpr auto value = V;

    template <typename T, typename A, typename ...B> using type = typename impl::dependent_type<T, A, B...>::type;
    template <typename A, typename ...B> using void_type = type<void, A, B...>;


    // Forces some of the function template parameters to be deduced.
    // Usage:` template <typename A, Meta::deduct..., typename B>` ...
    // All template parameters placed after `Meta::deduct...` can't be specified manually and would have to be deduced.

    namespace impl
    {
        class deduct_helper
        {
          protected:
            constexpr deduct_helper() {}
        };
    }

    using deduct = impl::deduct_helper &;


    // Lambda overloader.

    template <typename ...P> struct overload : P... { using P::operator()...; };
    template <typename ...P> overload(P...) -> overload<P...>;


    // Copy cv-qualifiers from one type to another.

    namespace impl
    {
        template <typename A, typename B> struct copy_qualifiers {using type = B;};
        template <typename A, typename B> struct copy_qualifiers<const          A, B> {using type = const          B;};
        template <typename A, typename B> struct copy_qualifiers<      volatile A, B> {using type =       volatile B;};
        template <typename A, typename B> struct copy_qualifiers<const volatile A, B> {using type = const volatile B;};
    }

    template <typename A, typename B> using copy_qualifiers = typename impl::copy_qualifiers<A, B>::type;


    // Pre-C++20 replacement for `std::is_detected_v`.

    namespace impl
    {
        template <typename DummyVoid, template <typename...> typename A, typename ...B> struct is_detected : std::false_type {};
        template <template <typename...> typename A, typename ...B> struct is_detected<void_type<A<B...>>, A, B...> : std::true_type {};
    }

    template <template <typename...> typename A, typename ...B> inline constexpr bool is_detected = impl::is_detected<void, A, B...>::value;


    // An object wrapper that moves the underlying object even when copied.
    // Also has a function-call operator that forwards the call to the underlying objects,
    // which makes it good for putting non-copyable lambdas into `std::function`s.

    template <typename T> struct fake_copyable
    {
        mutable T value;

        constexpr fake_copyable(T &&value = {}) : value(std::move(value)) {}

        constexpr fake_copyable(const fake_copyable &other) : value(std::move(other.value)) {}
        constexpr fake_copyable(fake_copyable &&other) : value(std::move(other.value)) {}

        constexpr fake_copyable &operator=(const fake_copyable &other) {value = std::move(other.value); return *this;}
        constexpr fake_copyable &operator=(fake_copyable &&other) {value = std::move(other.value); return *this;}

        template <typename ...P>
        decltype(auto) operator()(P &&... params)
        {
            return value(std::forward<P>(params)...);
        }
    };

    template <typename T> fake_copyable(T) -> fake_copyable<T>;


    // Constexpr replacement for the for loop.

    template <typename Integer, Integer ...I, typename F> constexpr void cexpr_for_each(std::integer_sequence<Integer, I...>, F &&func)
    {
        (func(std::integral_constant<Integer, I>{}) , ...);
    }

    template <auto N, typename F> constexpr void cexpr_for(F &&func)
    {
        if constexpr (N > 0)
        {
            cexpr_for_each(std::make_integer_sequence<decltype(N), N>{}, std::forward<F>(func));
        }
    }


    // Helper functions to generate sequences of values by invoking lambdas with constexpr indices as parameters.

    template <typename T, typename Integer, Integer ...I, typename F> constexpr auto cexpr_generate_from_seq(std::integer_sequence<Integer, I...>, F &&func)
    {
        return T{func(std::integral_constant<Integer, I>{})...};
    }
    template <template <typename> typename T, typename Integer, Integer ...I, typename F> constexpr auto cexpr_generate_from_seq(std::integer_sequence<Integer, I...>, F &&func)
    {
        return T{func(std::integral_constant<Integer, I>{})...};
    }
    template <typename Integer, Integer ...I, typename F> constexpr auto cexpr_generate_array_from_seq(std::integer_sequence<Integer, I...>, F &&func)
    {
        return std::array{func(std::integral_constant<Integer, I>{})...};
    }

    template <typename T, auto N, typename F> constexpr auto cexpr_generate(F &&func)
    {
        return cexpr_generate_from_seq<T>(std::make_integer_sequence<decltype(N), N>{}, std::forward<F>(func));
    }
    template <template <typename> typename T, auto N, typename F> constexpr auto cexpr_generate(F &&func)
    {
        return cexpr_generate_from_seq<T>(std::make_integer_sequence<decltype(N), N>{}, std::forward<F>(func));
    }
    template <auto N, typename F> constexpr auto cexpr_generate_array(F &&func)
    {
        return cexpr_generate_array_from_seq(std::make_integer_sequence<decltype(N), N>{}, std::forward<F>(func));
    }


    // Invoke a function with a constexpr-ized integer argument.
    // `func` is called with an argument of type `std::integral_constant<decltype(N), i>`, and its return value is returned.
    // `i` has to be in the range `0..N-1`, otherwise the behavior is undefined (likely a crash if the `std::array` bounds checking is disabled).

    template <auto N, typename F>
    [[nodiscard]] constexpr decltype(auto) with_cexpr_value(decltype(N) i, F &&func)
    {
        static_assert(N >= 1);

        return cexpr_generate_array<N>([&](auto value)
        {
            return +[](F &&func) -> decltype(auto)
            {
                return std::forward<F>(func)(decltype(value){});
            };
        })[i](std::forward<F>(func));
    }


    // Invoke a funciton with a set of constexpr-ized boolean flags.
    // (Beware that 2^n instantinations of the function will be generated.)
    // Example usage:
    //     T result = Meta::cexpr_flags(0,1,1) >> [](auto a, auto b, auto c) {return a.value + b.value + c.value};

    namespace impl
    {
        using cexpr_flag_bits_t = unsigned int;

        template <typename F, int ...I>
        constexpr decltype(auto) with_cexpr_flags(cexpr_flag_bits_t flags, F &&func, std::integer_sequence<int, I...>)
        {
            static_assert(sizeof...(I) <= sizeof(cexpr_flag_bits_t) * 8);
            constexpr cexpr_flag_bits_t func_count = (cexpr_flag_bits_t)1 << (cexpr_flag_bits_t)(sizeof...(I));
            return with_cexpr_value<func_count>(flags, [&](auto index) -> decltype(auto)
            {
                constexpr cexpr_flag_bits_t i = index;
                return std::forward<F>(func)(std::bool_constant<bool(i & cexpr_flag_bits_t(cexpr_flag_bits_t(1) << cexpr_flag_bits_t(I)))>{}...);
            });
        }

        template <typename L>
        class cexpr_flags_expr
        {
            L lambda;
          public:
            constexpr cexpr_flags_expr(L lambda) : lambda(lambda) {}

            template <typename F>
            constexpr decltype(auto) operator>>(F &&func) const
            {
                return lambda(std::forward<F>(func));
            }
        };
    }

    template <typename ...P, CHECK(std::is_convertible_v<const P &, bool> && ...)>
    [[nodiscard]] constexpr auto with_cexpr_flags(const P &... params)
    {
        impl::cexpr_flag_bits_t flags = 0, mask = 1;
        ((flags |= mask * bool(params), mask <<= 1) , ...);

        auto lambda = [flags](auto &&func) -> decltype(auto)
        {
            return impl::with_cexpr_flags(flags, decltype(func)(func), std::make_integer_sequence<int, sizeof...(P)>{});
        };

        return impl::cexpr_flags_expr(lambda);
    }


    // Conditionally copyable/movable base class.

    template <typename T, bool C> struct copyable_if {};

    template <typename T> struct copyable_if<T, 0>
    {
        // Here we use CRTP to make sure the empty base class optimization is never defeated.
        constexpr copyable_if() noexcept = default;
        ~copyable_if() noexcept = default;
        constexpr copyable_if(copyable_if &&) noexcept = default;
        constexpr copyable_if &operator=(copyable_if &&) noexcept = default;

        copyable_if(const copyable_if &) noexcept = delete;
        copyable_if &operator=(const copyable_if &) noexcept = delete;
    };


    // Non-copyable and non-movable base class.

    template <typename T> struct stationary // Use this as a CRTP base.
    {
        // Here we use CRTP to make sure the empty base class optimization is never defeated.
        constexpr stationary() noexcept = default;
        stationary(const stationary &) = delete;
        stationary(stationary &&) = delete;
        stationary &operator=(const stationary &) = delete;
        stationary &operator=(stationary &&) = delete;
        ~stationary() = default;
    };


    // Polymorphic base class.

    template <typename T> struct with_virtual_destructor // Use this as a CRTP base.
    {
        // The idea behind using CRTP is avoid connecting unrelated classes via a common base. I think we need to avoid that.
        constexpr with_virtual_destructor() noexcept = default;
        constexpr with_virtual_destructor(const with_virtual_destructor &) noexcept = default;
        constexpr with_virtual_destructor(with_virtual_destructor &&) noexcept = default;
        with_virtual_destructor &operator=(const with_virtual_destructor &) noexcept = default;
        with_virtual_destructor &operator=(with_virtual_destructor &&) noexcept = default;
        virtual ~with_virtual_destructor() = default;
    };
}
