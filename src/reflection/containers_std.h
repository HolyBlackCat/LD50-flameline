#pragma once

#include <utility>

#include "interface.h"

#include "utils/meta.h"

namespace Refl::Custom
{
    template <typename T> struct Container<T, std::void_t<typename T::value_type, typename T::size_type, decltype(std::declval<T&>().begin()), decltype(std::declval<T&>().end())>>
    {
        inline static const std::string name = "list";

        static constexpr std::size_t size(const T &object) {return object.size();}

        static constexpr auto begin(T &object) {return object.begin();}
        static constexpr auto end  (T &object) {return object.end();}
        static constexpr auto const_begin(const T &object) {return object.begin();}
        static constexpr auto const_end  (const T &object) {return object.end();}

        using element_type = typename T::value_type;

        template <typename A> using _has_single_arg_insert_impl = decltype(std::declval<A &>().insert(std::declval<element_type &>()));
        inline static constexpr bool _has_single_arg_insert = Meta::is_detected<_has_single_arg_insert_impl, T>;

        static constexpr void insert(T &object, const element_type &value)
        {
            if constexpr (_has_single_arg_insert)
                object.insert(value);
            else
                object.insert(object.end(), value);
        }
        static constexpr void insert_move(T &object, element_type &&value)
        {
            if constexpr (_has_single_arg_insert)
                object.insert(std::move(value));
            else
                object.insert(object.end(), std::move(value));
        }
    };
}
