#pragma once

#include <array>
#include <type_traits>
#include <utility>

namespace Meta
{
    template <typename T> struct tag {using type = T;};
    template <typename...> struct type_list {};
    template <auto...> struct value_list {};


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


    namespace impl
    {
        template <typename DummyVoid, template <typename...> typename A, typename ...B> struct is_detected : std::false_type {};
        template <template <typename...> typename A, typename ...B> struct is_detected<void_type<A<B...>>, A, B...> : std::true_type {};
    }

    template <template <typename...> typename A, typename ...B> inline constexpr bool is_detected = impl::is_detected<void, A, B...>::value;


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


    template <bool C> struct copyable_if {};

    template <> struct copyable_if<0>
    {
        copyable_if() = default;
        ~copyable_if() = default;
        copyable_if(copyable_if &&) = default;
        copyable_if &operator=(copyable_if &&) = default;

        copyable_if(const copyable_if &) = delete;
        copyable_if &operator=(const copyable_if &) = delete;
    };


    template <typename Base> struct polymorphic // Use this as a CRTP base.
    {
        constexpr polymorphic() noexcept = default;
        constexpr polymorphic(const polymorphic &) noexcept = default;
        constexpr polymorphic(polymorphic &&) noexcept = default;
        polymorphic &operator=(const polymorphic &) noexcept = default;
        polymorphic &operator=(polymorphic &&) noexcept = default;
        virtual ~polymorphic() = default;
    };
}
