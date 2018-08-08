#ifndef REFLECTION_CONTAINERS_STD_H_INCLUDED
#define REFLECTION_CONTAINERS_STD_H_INCLUDED

#include <utility>

#include "interface.h"

namespace Refl::Custom
{
    template <typename T> struct Container<T, std::void_t<typename T::value_type, typename T::size_type, decltype(std::declval<T&>().begin()), decltype(std::declval<T&>().end())>>
    {
        static constexpr std::size_t size(const T &object) {return object.size();}

        static constexpr auto begin(T &object) {return object.begin();}
        static constexpr auto end  (T &object) {return object.end();}
        static constexpr auto const_begin(const T &object) {return object.begin();}
        static constexpr auto const_end  (const T &object) {return object.end();}
    };
}

#endif
