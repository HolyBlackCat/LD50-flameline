#pragma once

#include <string>
#include <type_traits>

#include "graphics/texture.h"
#include "utils/mat.h"

namespace Graphics
{
    // GLSL support.

    template <typename T> std::string GlslTypeName()
    {
        if constexpr (Math::is_vector_v<T> || Math::is_matrix_v<T>)
        {
            constexpr int is_vec = Math::is_vector_v<T>;
            using base = typename T::type;

            std::string ret;

                 if constexpr (std::is_same_v<base, bool        >) ret = "b";
            else if constexpr (std::is_same_v<base, double      >) ret = "d";
            else if constexpr (std::is_same_v<base, int         >) ret = "i";
            else if constexpr (std::is_same_v<base, unsigned int>) ret = "u";
            else static_assert(std::is_same_v<base, float>, "No name for this type.");

            if constexpr (is_vec)
            {
                ret += "vec";
                ret += std::to_string(T::size);
            }
            else
            {
                ret += "mat";
                ret += std::to_string(T::width);
                if constexpr(T::width != T::height)
                {
                    ret += "x";
                    ret += std::to_string(T::height);
                }
            }

            return ret;
        }
        else if constexpr (std::is_same_v<T, TexUnit     >) return "sampler2D";
        else if constexpr (std::is_same_v<T, bool        >) return "bool";
        else if constexpr (std::is_same_v<T, float       >) return "float";
        else if constexpr (std::is_same_v<T, double      >) return "double";
        else if constexpr (std::is_same_v<T, int         >) return "int";
        else if constexpr (std::is_same_v<T, unsigned int>) return "uint";
        else static_assert(!sizeof(T), "No name for this type.");
    }
}
