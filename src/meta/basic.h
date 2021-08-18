#pragma once

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
}
