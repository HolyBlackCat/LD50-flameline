#pragma once

#include <array>
#include <type_traits>
#include <utility>

#include "macros/check.h"

namespace Meta
{
    // Tag dispatch helpers.

    template <typename T> struct tag {using type = T;};
    template <auto V> struct value_tag {static constexpr auto value = V;};
    template <typename...> struct type_list {};
    template <auto...> struct value_list {};


    // Some helper aliases.

    template <typename T> using identity_t = T;


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


    // Utilities for manipulating `{type|value}_list`s.

    namespace impl
    {
        // Returns size of a `type_list` or `value_list`.
        template <typename T> struct list_size {};
        template <typename ...P> struct list_size<type_list<P...>> : std::integral_constant<std::size_t, sizeof...(P)> {};
        template <auto ...V> struct list_size<value_list<V...>> : std::integral_constant<std::size_t, sizeof...(V)> {};

        // Concat two lists.
        template <typename A, typename B> struct list_cat {};
        template <typename ...A, typename ...B> struct list_cat<type_list<A...>, type_list<B...>> {using type = type_list<A..., B...>;};
        template <auto ...A, auto ...B> struct list_cat<value_list<A...>, value_list<B...>> {using type = value_list<A..., B...>;};

        // Return ith element of a list.
        template <typename T, std::size_t I> struct list_at {};

        template <typename T, typename ...P, std::size_t I> struct list_at<type_list<T, P...>, I>
        {
            using type = typename list_at<type_list<P...>, I-1>::type;
        };
        template <typename T, typename ...P> struct list_at<type_list<T, P...>, 0>
        {
            using type = T;
        };
        template <std::size_t I> struct list_at<type_list<>, I>
        {
            static_assert(value<false, value_tag<I>>, "List index is out of range.");
        };

        template <auto V, auto ...P, std::size_t I> struct list_at<value_list<V, P...>, I>
        {
            static constexpr auto value = list_at<value_list<P...>, I-1>::value;
        };
        template <auto V, auto ...P> struct list_at<value_list<V, P...>, 0>
        {
            static constexpr auto value = V;
        };
        template <std::size_t I> struct list_at<value_list<>, I>
        {
            static_assert(value<false, value_tag<I>>, "List index is out of range.");
        };

        // Check if a list contains a value.
        template <typename L, typename T> struct list_contains_type {};
        template <typename ...P, typename T> struct list_contains_type<type_list<P...>, T> : std::bool_constant<(std::is_same_v<P, T> || ...)> {};

        template <typename L, auto V> struct list_contains_value {};
        template <auto ...P, auto V> struct list_contains_value<value_list<P...>, V> : std::bool_constant<((P == V) || ...)> {};

        // Return index of the first occurence of an element in a list, abort if not found.
        template <typename L, typename T> struct list_type_index {};
        template <typename T> struct list_type_index<type_list<>, T> {static_assert(value<false, T>, "No such type in the list.");};
        template <typename F, typename ...P, typename T> struct list_type_index<type_list<F, P...>, T> {static constexpr std::size_t value = 1+list_type_index<type_list<P...>,T>::value;};
        template <typename F, typename ...P> struct list_type_index<type_list<F, P...>, F> {static constexpr std::size_t value = 0;};

        template <typename L, auto V> struct list_value_index {};
        template <auto V> struct list_value_index<value_list<>, V> {static_assert(value<false, value_tag<V>>, "No such type in the list.");};
        template <auto F, auto ...P, auto V> struct list_value_index<value_list<F, P...>, V> {static constexpr std::size_t value = 1+list_value_index<value_list<P...>,V>::value;};
        template <auto F, auto ...P> struct list_value_index<value_list<F, P...>, F> {static constexpr std::size_t value = 0;};

        // For each element of A, append it to B if it's not already in B.
        template <typename A, typename B> struct list_copy_uniq {};
        template <typename B> struct list_copy_uniq<type_list<>, B> {using type = B;};
        template <typename B> struct list_copy_uniq<value_list<>, B> {using type = B;};
        template <typename A1, typename ...A, typename ...B> struct list_copy_uniq<type_list<A1, A...>, type_list<B...>>
        {
            using type = typename list_copy_uniq<type_list<A...>, std::conditional_t<(std::is_same_v<A1, B> || ...), type_list<B...>, type_list<B..., A1>>>::type;
        };
        template <auto A1, auto ...A, auto ...B> struct list_copy_uniq<value_list<A1, A...>, value_list<B...>>
        {
            using type = typename list_copy_uniq<value_list<A...>, std::conditional_t<((A1 == B) || ...), value_list<B...>, value_list<B..., A1>>>::type;
        };

        // Remove duplicate elements from a list.
        template <typename T> struct list_uniq {};
        template <typename ...P> struct list_uniq<type_list<P...>> {using type = typename list_copy_uniq<type_list<P...>, type_list<>>::type;};
        template <auto ...P> struct list_uniq<value_list<P...>> {using type = typename list_copy_uniq<value_list<P...>, value_list<>>::type;};
    }

    // Returns size of a `{type|value}_list`.
    template <typename T> inline constexpr std::size_t list_size = impl::list_size<T>::value;

    template <typename A, typename B> using list_cat = typename impl::list_cat<A, B>::type;

    // Return ith element of a list.
    template <typename T, std::size_t I> using list_type_at = typename impl::list_at<T, I>::type;
    template <typename T, std::size_t I> inline constexpr auto list_value_at = impl::list_at<T, I>::value;

    // Check if a list contains an element.
    template <typename L, typename T> inline constexpr bool list_contains_type = impl::list_contains_type<L, T>::value;
    template <typename L, auto V> inline constexpr bool list_contains_value = impl::list_contains_value<L, V>::value;

    // Return the index of the first occurence of the element in the list.
    template <typename L, typename T> inline constexpr std::size_t list_type_index = impl::list_type_index<L, T>::value;
    template <typename L, auto V> inline constexpr std::size_t list_value_index = impl::list_value_index<L, V>::value;

    // Add all elements from `A` that don't exist in `B` to `B`.
    template <typename A, typename B> using list_copy_uniq = typename impl::list_copy_uniq<A, B>::type;
    // Remove duplicate elements from a list.
    template <typename T> using list_uniq = typename impl::list_uniq<T>::type;


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


    // Copy cv-qualifiers from one type to another.

    namespace impl
    {
        template <typename A, typename B> struct copy_qualifiers {using type = B;};
        template <typename A, typename B> struct copy_qualifiers<const          A, B> {using type = const          B;};
        template <typename A, typename B> struct copy_qualifiers<      volatile A, B> {using type =       volatile B;};
        template <typename A, typename B> struct copy_qualifiers<const volatile A, B> {using type = const volatile B;};
    }

    template <typename A, typename B> using copy_qualifiers = typename impl::copy_qualifiers<A, B>::type;


    // A replacement for `std::experimental::is_detected`.

    namespace impl
    {
        template <typename DummyVoid, template <typename...> typename A, typename ...B> struct is_detected : std::false_type {};
        template <template <typename...> typename A, typename ...B> struct is_detected<void_type<A<B...>>, A, B...> : std::true_type {};
    }

    template <template <typename...> typename A, typename ...B> inline constexpr bool is_detected = impl::is_detected<void, A, B...>::value;


    // Checks if a type is a specialization of a template.
    // `is_specialization_of<A, B>` is true if `A` is `B<P...>`, where `P...` are some types.

    namespace impl
    {
        template <typename A, template <typename...> typename B>
        struct is_specialization_of : std::false_type {};

        template <template <typename...> typename T, typename ...P>
        struct is_specialization_of<T<P...>, T> : std::true_type {};
    }

    template <typename A, template <typename...> typename B>
    inline constexpr bool is_specialization_of = impl::is_specialization_of<A, B>::value;


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


    // Checks if A is the same type as B, or if A is `void`.

    template <typename A, typename B> inline constexpr bool is_same_or_void_v = std::is_void_v<A> || std::is_same_v<A, B>;


    // A helper function that invokes a callback.
    // It expects the callback to either have the same return type as the type of the first parameter, or void. Otherwise a static assertion is triggered.
    // The callback is invoked, and if returns a non-void type, its return value is returned from this function.
    // Otherwise the value of the first parameter is returned.

    template <deduce..., typename F, typename R, typename ...P>
    R invoke_and_get_return_value_or(const R &default_val, F &&func, P &&... params)
    {
        using ret_t = decltype(std::forward<F>(func)(std::forward<P>(params)...));
        static_assert(is_same_or_void_v<ret_t, R>, "The return type of the callback must either be void or match the type of the first parameter.");
        if constexpr (std::is_void_v<ret_t>)
        {
            std::forward<F>(func)(std::forward<P>(params)...);
            return default_val;
        }
        else
        {
            // The cast is necessary if `R` is an lvalue reference.
            return static_cast<R>(std::forward<F>(func)(std::forward<P>(params)...));
        }
    }


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
