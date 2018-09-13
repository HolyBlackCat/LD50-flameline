#pragma once

#include <string>
#include <type_traits>

#include "interface.h"

namespace Refl::Custom
{
    template <typename T> struct Structure<T, std::enable_if_t<std::is_array_v<T>>>
    {
        // Field indices are guaranteed to be in valid range.
        inline static const std::string name = "array?";

        static constexpr bool is_tuple = 1;
        static constexpr int field_count = std::extent_v<T>;
        template <int I> static constexpr auto &field(T &object)
        {
            return object[I];
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
