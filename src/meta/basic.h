#pragma once

namespace Meta
{
    // Tag dispatch helpers.

    template <typename T> struct tag {using type = T;};
    template <auto V> struct value_tag {static constexpr auto value = V;};


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
}
