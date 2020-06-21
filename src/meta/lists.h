#pragma once

#include "meta/basic.h" // For `Meta::value`.

namespace Meta
{
    template <typename...> struct type_list {};
    template <auto...> struct value_list {};


    namespace impl
    {
        // Substitute elements of a list into a template.
        template <template <typename...> typename T, typename L> struct list_apply_types {};
        template <template <typename...> typename T, typename ...P> struct list_apply_types<T, type_list<P...>> {using type = T<P...>;};
        template <template <auto...> typename T, typename L> struct list_apply_values {};
        template <template <auto...> typename T, auto ...P> struct list_apply_values<T, value_list<P...>> {using type = T<P...>;};

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

        // Check if a list is a subset of another one.
        template <typename A, typename B> struct list_is_subset_of {};
        template <typename ...A, typename B> struct list_is_subset_of<type_list<A...>, B> : std::bool_constant<(list_contains_type<B, A>::value && ...)> {};
        template <auto ...A, typename B> struct list_is_subset_of<value_list<A...>, B> : std::bool_constant<(list_contains_value<B, A>::value && ...)> {};

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

    // Substitutes elements of a list into a type template;
    template <template <typename...> typename T, typename L> using list_apply_types = typename impl::list_apply_types<T, L>::type;
    template <template <auto...> typename T, typename L> using list_apply_values = typename impl::list_apply_values<T, L>::type;

    // Returns size of a `{type|value}_list`.
    template <typename T> inline constexpr std::size_t list_size = impl::list_size<T>::value;

    // Concatenates two lists.
    template <typename A, typename B> using list_cat = typename impl::list_cat<A, B>::type;

    // Return ith element of a list.
    template <typename T, std::size_t I> using list_type_at = typename impl::list_at<T, I>::type;
    template <typename T, std::size_t I> inline constexpr auto list_value_at = impl::list_at<T, I>::value;

    // Check if a list contains an element.
    template <typename L, typename T> inline constexpr bool list_contains_type = impl::list_contains_type<L, T>::value;
    template <typename L, auto V> inline constexpr bool list_contains_value = impl::list_contains_value<L, V>::value;

    // Check if a list is a subset of a different list.
    template <typename A, typename B> inline constexpr bool list_is_subset_of = impl::list_is_subset_of<A, B>::value;

    // Return the index of the first occurence of the element in the list.
    template <typename L, typename T> inline constexpr std::size_t list_type_index = impl::list_type_index<L, T>::value;
    template <typename L, auto V> inline constexpr std::size_t list_value_index = impl::list_value_index<L, V>::value;

    // Add all elements from `A` that don't exist in `B` to `B`.
    template <typename A, typename B> using list_copy_uniq = typename impl::list_copy_uniq<A, B>::type;
    // Remove duplicate elements from a list.
    template <typename T> using list_uniq = typename impl::list_uniq<T>::type;
}
