#pragma once

#include <array>
#include <string>

#include "interface.h"

#include "utils/mat.h"

namespace Refl::Custom
{
    template <typename T> struct Structure<T, std::enable_if_t<Math::is_vector_v<T>>>
    {
        // Field indices are guaranteed to be in valid range.
        inline static const std::string name = "vec" + std::to_string(T::size) + "<" + Refl::Interface<typename T::type>::name() + ">";

        static constexpr bool is_tuple = 1;
        static constexpr int field_count = T::size;
        template <int I> static constexpr auto &field(T &object)
        {
            if constexpr (I == 0) return object.x;
            if constexpr (I == 1) return object.y;
            if constexpr (I == 2) return object.z;
            if constexpr (I == 3) return object.w;
        }
        static std::string field_name(int index)
        {
            return std::array<const char *, 4>{"x","y","z","w"}[index];
        }
        static constexpr FieldCategory field_category(int index)
        {
            (void)index;
            return FieldCategory::mandatory;
        }
    };

    template <typename T> struct Structure<T, std::enable_if_t<Math::is_matrix_v<T>>>
    {
        // Field indices are guaranteed to be in valid range.
        inline static const std::string name = "mat" + std::to_string(T::width) + "x" + std::to_string(T::width) + "<" + Refl::Interface<typename T::type>::name() + ">";

        static constexpr bool is_tuple = 1;
        static constexpr int field_count = T::width * T::height;
        template <int I> static constexpr auto &field(T &object)
        {
            constexpr int x = I % T::width;
            constexpr int y = I / T::width;

            if constexpr (y == 0)
            {
                if constexpr (x == 0) return object.x.x;
                if constexpr (x == 1) return object.y.x;
                if constexpr (x == 2) return object.z.x;
                if constexpr (x == 3) return object.w.x;
            }
            else if constexpr (y == 1)
            {
                if constexpr (x == 0) return object.x.y;
                if constexpr (x == 1) return object.y.y;
                if constexpr (x == 2) return object.z.y;
                if constexpr (x == 3) return object.w.y;
            }
            else if constexpr (y == 2)
            {
                if constexpr (x == 0) return object.x.z;
                if constexpr (x == 1) return object.y.z;
                if constexpr (x == 2) return object.z.z;
                if constexpr (x == 3) return object.w.z;
            }
            else if constexpr (y == 3)
            {
                if constexpr (x == 0) return object.x.w;
                if constexpr (x == 1) return object.y.w;
                if constexpr (x == 2) return object.z.w;
                if constexpr (x == 3) return object.w.w;
            }
        }
        static std::string field_name(int index)
        {
            int x = index % T::width;
            int y = index / T::width;
            std::array<const char *, 4> names{"x","y","z","w"};
            return std::string(names[x]) + "." + names[y];
        }
        static constexpr FieldCategory field_category(int index)
        {
            (void)index;
            return FieldCategory::mandatory;
        }
    };
}
