#ifndef REFLECTION_STRUCTURES_VEC_MAT_H_INCLUDED
#define REFLECTION_STRUCTURES_VEC_MAT_H_INCLUDED

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
            return std::array<std::string,4>{"x","y","z","w"}[index];
        }
        static constexpr FieldCategory field_category(int index)
        {
            (void)index;
            return FieldCategory::mandatory;
        }
    };
}

#endif
