#pragma once

namespace Meta
{
    // Tag dispatch helpers.

    template <typename T> struct tag {using type = T;};
    template <auto V> struct value_tag {static constexpr auto value = V;};
}
