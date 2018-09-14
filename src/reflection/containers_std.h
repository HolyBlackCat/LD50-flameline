#pragma once

#include <cstddef>
#include <string>
#include <utility>

#include "interface.h"

#include "utils/meta.h"

namespace Refl::Custom
{
    namespace impl
    {
        template <typename T> inline constexpr bool assume_not_std_container = 0;
        template <typename T, std::size_t N> inline constexpr bool assume_not_std_container<std::array<T,N>> = 1;
        template <> inline constexpr bool assume_not_std_container<std::string> = 1;
    }

    template <typename T> struct Container
    <
        T,
        std::void_t
        <
            std::enable_if_t<!impl::assume_not_std_container<T>>,
            typename T::value_type,
            typename T::size_type,
            decltype(std::declval<T&>().begin()),
            decltype(std::declval<T&>().end())
        >
    >
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

        // Those return 1 on success.
        static constexpr bool insert(T &object, const element_type &value)
        {
            if constexpr (_has_single_arg_insert)
            {
                return object.insert(value).second;
            }
            else
            {
                object.insert(object.end(), value);
                return true;
            }
        }
        static constexpr bool insert_move(T &object, element_type &&value)
        {
            if constexpr (_has_single_arg_insert)
            {
                return object.insert(std::move(value)).second;
            }
            else
            {
                object.insert(object.end(), std::move(value));
                return true;
            }
        }
    };
}
