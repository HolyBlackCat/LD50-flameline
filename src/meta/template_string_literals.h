#pragma once

#include <algorithm>
#include <cstddef>

#include "meta/basic.h"

// This file offers compile-time strings that can be used as template parameters.
// Example 1:
//     template <Meta::StringParam Name> void foo() {std::cout << Name.str << '\n';}
//     foo<"123">();
// Example 2:
//     template <Meta::StringParam Name> void foo(Meta::value_tag<Name>) {std::cout << Name.str << '\n';}
//     foo("123"_l);

namespace Meta
{
    // A string that can be used as a template parameter.
    template <std::size_t N>
    struct StringParam
    {
        static constexpr std::size_t size = N - 1;
        char str[size]{};

        constexpr StringParam() {}
        constexpr StringParam(const char (&new_str)[N])
        {
            ASSERT(new_str[N-1] == '\0');
            std::copy_n(new_str, size, str);
        }
    };

    template <Meta::StringParam S>
    constexpr Meta::value_tag<S> operator""_l()
    {
        return {};
    }
}

using Meta::operator""_l;
