#pragma once

#include <algorithm>
#include <cstddef>

#include "meta/basic.h"
#include "program/errors.h"

// This file offers compile-time strings that can be used as template parameters.
// Example 1:
//     template <Meta::ConstString Name> void foo() {std::cout << Name.str << '\n';}
//     foo<"123">();
// Example 2:
//     template <Meta::ConstString Name> void foo(Meta::ConstStringParam<Name>) {std::cout << Name.str << '\n';}
//     foo("123"_c);

namespace Meta
{
    // A string that can be used as a template parameter.
    template <std::size_t N>
    struct ConstString
    {
        char str[N]{};

        static constexpr std::size_t size = N - 1;

        [[nodiscard]] std::string_view view() const
        {
            return {str, str + size};
        }

        constexpr ConstString() {}
        constexpr ConstString(const char (&new_str)[N])
        {
            ASSERT(new_str[N-1] == '\0');
            std::copy_n(new_str, N, str);
        }
    };

    // A tag structure returned by `operator""_c` below.
    template <Meta::ConstString S>
    struct ConstStringParam {};

    // Returns a string encoded into a template parameter of a tag structure `ConstStringParam`.
    template <Meta::ConstString S>
    [[nodiscard]] constexpr ConstStringParam<S> operator""_c()
    {
        return {};
    }
}

using Meta::operator""_c;
