#pragma once

#include <type_traits>
#include <utility>

namespace Meta
{
    template <typename T> struct tag {using type = T;};


    namespace impl
    {
        template <typename DummyVoid, template <typename...> typename A, typename ...B> struct is_detected : std::false_type {};
        template <template <typename...> typename A, typename ...B> struct is_detected<std::void_t<A<B...>>, A, B...> : std::true_type {};
    }

    template <template <typename...> typename A, typename ...B> inline constexpr bool is_detected = impl::is_detected<void, A, B...>::value;


    template <int N, typename F> constexpr void cexpr_for(F &&func)
    {
        if constexpr (N > 0)
        {
            [&]<int ...I>(std::integer_sequence<int, I...>)
            {
                (func(std::integral_constant<int, I>{}) , ...);
            }
            (std::make_integer_sequence<int, N>{});
        }
    }


    template <auto V, typename> inline constexpr auto always = V;
}
