#pragma once

#include <algorithm>
#include <cstddef>

#include "meta/basic.h"

// This file offers compile-time strings that can be used as template parameters.
// Example 1:
//     template <Meta::ConstString Name> void foo() {std::cout << Name.str << '\n';}
//     foo<"123">();
// Example 2:
//     template <Meta::ConstString Name> void foo(Meta::ConstStringParam<Name>) {std::cout << Name.str << '\n';}
//     foo("123"_c);

namespace Meta
{
    namespace impl
    {
        // Does nothing, but causes an error if called from a `consteval` function.
        inline void expectedNullTerminatedArray() {}
    }

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

        consteval ConstString() {}
        consteval ConstString(const char (&new_str)[N])
        {
            if (new_str[N-1] != '\0')
                impl::expectedNullTerminatedArray();
            std::copy_n(new_str, size, str);
        }
    };

    template <std::size_t A, std::size_t B>
    [[nodiscard]] constexpr ConstString<A + B - 1> operator+(const ConstString<A> &a, const ConstString<B> &b)
    {
        ConstString<A + B - 1> ret;
        std::copy_n(a.str, a.size, ret.str);
        std::copy_n(b.str, b.size, ret.str + a.size);
        return ret;
    }

    template <std::size_t A, std::size_t B>
    [[nodiscard]] constexpr ConstString<A + B - 1> operator+(const ConstString<A> &a, const char (&b)[B])
    {
        return a + ConstString<B>(b);
    }

    template <std::size_t A, std::size_t B>
    [[nodiscard]] constexpr ConstString<A + B - 1> operator+(const char (&a)[A], const ConstString<B> &b)
    {
        return ConstString<A>(a) + b;
    }


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
