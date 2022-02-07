#pragma once

#include <array>
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace Meta
{
    // Tag dispatch helpers.

    template <typename T> struct tag {using type = T;};
    template <auto V> struct value_tag {static constexpr auto value = V;};


    // Some helper aliases.

    template <typename T> using identity_t = T;


    // Some concepts.

    template <typename T> concept cv_unqualified = std::is_same_v<std::remove_cv_t<T>, T>;
    template <typename T> concept cvref_unqualified = std::is_same_v<std::remove_cvref_t<T>, T>;


    // Dependent values and types, good for `static_assert`s and SFINAE.

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
    // Usage:` template <typename A, Meta::deduce..., typename B>` ...
    // All template parameters placed after `Meta::deduce...` can't be specified manually and would have to be deduced.

    namespace impl
    {
        class deduce_helper
        {
          protected:
            constexpr deduce_helper() {}
        };
    }

    using deduce = impl::deduce_helper &;


    // Lambda overloader.

    template <typename ...P> struct overload : P... { using P::operator()...; };
    template <typename ...P> overload(P...) -> overload<P...>;


    // Checks if a type is a specialization of a template.
    // `specialization_of<A, B>` is true if `A` is `B<P...>`, where `P...` are some types.

    namespace impl
    {
        template <typename A, template <typename...> typename B>
        struct specialization_of : std::false_type {};

        template <template <typename...> typename T, typename ...P>
        struct specialization_of<T<P...>, T> : std::true_type {};
    }

    template <typename A, template <typename...> typename B>
    concept specialization_of = impl::specialization_of<A, B>::value;


    // A helper function that invokes a callback.
    // If the callback returns void, the function returns `default_var` explicitly converted to `R`.
    // Otherwise returns the return value of of the callback, explicitly converted to `R` .

    template <typename R, deduce..., typename D, typename F, typename ...P>
    R return_value_or(D &&default_val, F &&func, P &&... params)
    requires
        std::is_constructible_v<R, D> &&
        requires{requires std::is_void_v<std::invoke_result_t<F, P...>> || std::is_constructible_v<std::invoke_result_t<F, P...>, R>;}
    {
        if constexpr (std::is_void_v<std::invoke_result_t<F, P...>>)
        {
            std::invoke(std::forward<F>(func), std::forward<P>(params)...);
            return R(std::forward<D>(default_val));
        }
        else
        {
            return R(std::invoke(std::forward<F>(func), std::forward<P>(params)...));
        }
    }


    // A wrapper that resets the underlying object to the default-constructed value.
    // The wrapper is copyable, and the value is not changed on copy.

    template <typename T>
    struct ResetIfMovedFrom
    {
        T value{};

        constexpr ResetIfMovedFrom() {}
        constexpr ResetIfMovedFrom(const T &value) : value(value) {}
        constexpr ResetIfMovedFrom(T &&value) : value(std::move(value)) {}

        constexpr ResetIfMovedFrom(const ResetIfMovedFrom &) = default;
        constexpr ResetIfMovedFrom &operator=(const ResetIfMovedFrom &) = default;

        constexpr ResetIfMovedFrom(ResetIfMovedFrom &&other) noexcept
            : value(std::move(other.value))
        {
            other.value = T{};
        }
        constexpr ResetIfMovedFrom &operator=(ResetIfMovedFrom &&other) noexcept
        {
            if (&other == this)
                return *this;
            value = std::move(other.value);
            other.value = T{};
            return *this;
        }
    };


    // Copy qualifiers from one type to another. Original qualifiers are stripped.
    // Can copy either cv-qualifiers or cvref-qualifiers. In the former case, ref-qualifiers are not stripped from the destination.

    namespace impl
    {
        template <typename A, typename B> struct copy_cv_qualifiers                      {using type =                std::remove_cv_t<B>;};
        template <typename A, typename B> struct copy_cv_qualifiers<const          A, B> {using type = const          std::remove_cv_t<B>;};
        template <typename A, typename B> struct copy_cv_qualifiers<      volatile A, B> {using type =       volatile std::remove_cv_t<B>;};
        template <typename A, typename B> struct copy_cv_qualifiers<const volatile A, B> {using type = const volatile std::remove_cv_t<B>;};

        template <typename A, typename B> struct copy_cvref_qualifiers          {using type = typename copy_cv_qualifiers<A, std::remove_reference_t<B>>::type;};
        template <typename A, typename B> struct copy_cvref_qualifiers<A & , B> {using type = typename copy_cv_qualifiers<A, std::remove_reference_t<B>>::type &;};
        template <typename A, typename B> struct copy_cvref_qualifiers<A &&, B> {using type = typename copy_cv_qualifiers<A, std::remove_reference_t<B>>::type &&;};
    }

    template <typename A, typename B> using copy_cv_qualifiers = typename impl::copy_cv_qualifiers<A, B>::type;
    template <typename A, typename B> using copy_cvref_qualifiers = typename impl::copy_cvref_qualifiers<A, B>::type;


    // A replacement for `std::experimental::is_detected`.

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

        constexpr fake_copyable() {}
        constexpr fake_copyable(const T &value) : value(value) {}
        constexpr fake_copyable(T &&value) : value(std::move(value)) {}

        constexpr fake_copyable(const fake_copyable &other) : value(std::move(other.value)) {}
        constexpr fake_copyable(fake_copyable &&other) : value(std::move(other.value)) {}

        constexpr fake_copyable &operator=(const fake_copyable &other) {value = std::move(other.value); return *this;}
        constexpr fake_copyable &operator=(fake_copyable &&other) {value = std::move(other.value); return *this;}

        template <typename ...P>
        decltype(auto) operator()(P &&... params)
        {
            return value(std::forward<P>(params)...);
        }
        template <typename ...P>
        decltype(auto) operator()(P &&... params) const
        {
            return value(std::forward<P>(params)...);
        }
    };

    template <typename T> fake_copyable(T) -> fake_copyable<T>;


    // Checks if `T` is the same type as any of the `P...`.

    template <typename T, typename ...P> concept same_as_any_of = (std::same_as<T, P> || ...);


    // Constexpr replacement for the for loop.

    template <typename Integer, Integer ...I, typename F> constexpr void cexpr_for_each(std::integer_sequence<Integer, I...>, F &&func)
    {
        (func(std::integral_constant<Integer, I>{}) , ...);
    }
    template <typename Integer, Integer ...I, typename F> constexpr bool cexpr_any_of(std::integer_sequence<Integer, I...>, F &&func)
    {
        return (func(std::integral_constant<Integer, I>{}) || ...);
    }
    template <typename Integer, Integer ...I, typename F> constexpr bool cexpr_all_of(std::integer_sequence<Integer, I...>, F &&func)
    {
        return (func(std::integral_constant<Integer, I>{}) && ...);
    }

    template <auto N, typename F> constexpr void cexpr_for(F &&func)
    {
        if constexpr (N > 0)
            cexpr_for_each(std::make_integer_sequence<decltype(N), N>{}, std::forward<F>(func));
    }
    template <auto N, typename F> constexpr bool cexpr_any(F &&func)
    {
        if constexpr (N > 0)
            return cexpr_any_of(std::make_integer_sequence<decltype(N), N>{}, std::forward<F>(func));
        else
            return false;
    }
    template <auto N, typename F> constexpr bool cexpr_all(F &&func)
    {
        if constexpr (N > 0)
            return cexpr_all_of(std::make_integer_sequence<decltype(N), N>{}, std::forward<F>(func));
        else
            return true; // Note vacuous truth, unlike in `cexpr_any`.
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
        if constexpr (N <= 0)
        {
            (void)i;
            (void)func;
        }
        else
        {
            return cexpr_generate_array<N>([&](auto value)
            {
                return +[](F &&func) -> decltype(auto)
                {
                    return std::forward<F>(func)(decltype(value){});
                };
            })[i](std::forward<F>(func));
        }
    }


    // Invoke a funciton with a set of constexpr-ized boolean flags.
    // (Beware that 2^n instantinations of the function will be generated.)
    // Example usage:
    //     T result = Meta::with_cexpr_flags(0,1,1) >> [](auto a, auto b, auto c) {return a.value + b.value + c.value};

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

    template <typename ...P> requires (std::is_convertible_v<const P &, bool> && ...)
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
