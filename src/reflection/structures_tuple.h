#pragma once

#include <string>
#include <tuple>
#include <type_traits>

#include "interface.h"

namespace Refl::Custom
{
    // This specialization matches `std::tuple`, `std::array`, `std::pair`, and whatever else works with `std::get` and `std::tuple_size`.
    template <typename T> struct Structure<T, Meta::void_type<decltype(std::tuple_size<T>::value)>> // Note that we can't use `std::tuple_size_v` here, because it's not SFINAE-friendly (at least on GCC).
    {
        // Field indices are guaranteed to be in valid range.
        inline static const std::string name = "tuple?";

        static constexpr bool is_tuple = 1;
        static constexpr int field_count = std::tuple_size_v<T>;
        template <int I> static constexpr auto &field(T &object)
        {
            return std::get<I>(object);
        }
        static std::string field_name(int index)
        {
            return std::to_string(index);
        }
        static constexpr FieldCategory field_category(int index)
        {
            (void)index;
            return FieldCategory::mandatory;
        }
    };
}
