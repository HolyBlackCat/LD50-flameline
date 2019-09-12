#pragma once

#include "graphics/blending.h"
#include "graphics/geometry.h"
#include "graphics/shader.h"
#include "program/errors.h"
#include "reflection/complete.h"
#include "utils/mat.h"

namespace Graphics::Renderers
{
    template <typename T> struct FlatTransformation
    {
        static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "The base type can't have CV-qualifiers.");
        static_assert(!std::is_reference_v<T>, "The base type can't be a reference.");

        using type = T;
        using vector_t = vec2<T>;
        using matrix_t = mat3x2<T>;
        using small_matrix_t = mat2<T>;
        using large_matrix_t = mat3<T>;

        matrix_t matrix;

        constexpr FlatTransformation() {}
        constexpr FlatTransformation(const matrix_t &matrix) : matrix(matrix) {}
        constexpr FlatTransformation(const small_matrix_t &matrix) : matrix(matrix.to_mat3x2()) {}
        constexpr FlatTransformation(const large_matrix_t &matrix) : matrix(matrix.to_mat3x2()) {}

        FlatTransformation operator*(const FlatTransformation &other) const
        {
            return matrix(other.matrix);
        }
        FlatTransformation &operator*=(const FlatTransformation &other)
        {
            return *this = *this * other;
        }

        FlatTransformation translate(vector_t vec) const
        {
            FlatTransformation ret = *this;
            ret.matrix.z.x += vec.x * matrix.x.x + vec.y * matrix.y.x;
            ret.matrix.z.y += vec.x * matrix.x.y + vec.y * matrix.y.y;
            return ret;
        }
        FlatTransformation scale(type val) const
        {
            return scale(vector_t(val));
        }
        FlatTransformation scale(vector_t vec) const
        {
            FlatTransformation ret = *this;
            ret.matrix.x *= vec.x;
            ret.matrix.y *= vec.y;
            return ret;
        }
        FlatTransformation mirror_x(bool mirror = true) const
        {
            FlatTransformation ret = *this;
            if (mirror)
                ret.matrix.x *= -1;
            return ret;
        }
        FlatTransformation mirror_y(bool mirror = true) const
        {
            FlatTransformation ret = *this;
            if (mirror)
                ret.matrix.y *= -1;
            return ret;
        }
        FlatTransformation rotate(type angle) const
        {
            return add_matrix(small_matrix_t::rotate(angle));
        }
        FlatTransformation rotate_scale(vector_t vec) const
        {
            return add_matrix(small_matrix_t(vec, vec.rot90()));
        }
        FlatTransformation add_matrix(const matrix_t &new_mat) const
        {
            return FlatTransformation(matrix * new_mat.to_mat3());
        }
        FlatTransformation add_matrix(const small_matrix_t &new_mat) const
        {
            matrix_t ret_mat = (matrix.to_mat2() * new_mat).to_mat3();
            ret_mat.z = matrix.z;
            return FlatTransformation(ret_mat);
        }
        FlatTransformation add_matrix(const large_matrix_t &new_mat) const // The perspective-related part of the matrix (aka the last row) is ignored.
        {
            return add_matrix(new_mat.to_mat3x2());
        }
    };

    class Flat
    {
      public:
        struct Vertex
        {
            // factors.x: 0 - use color, 255 - use texture
            // factors.y: 0 - use color.a as alpha, 255 - use alpha from texture
            // factors.z: 0 - additive blending, 255 - regular blending

            // Note that fields are declared in a weird order to improve packing.
            Reflect(Vertex)
            (
                (fvec2)(pos)(={}),
                (fvec2)(texcoord)(={}),
                (u8vec4)(color)(={}),
                (u8vec3)(factors)(={}),
            )

            Vertex() {}

            Vertex(fvec2 pos, u8vec4 color, fvec2 texcoord, u8vec3 factors)
                : pos(pos), texcoord(texcoord), color(color), factors(factors)
            {}

            [[nodiscard]] static Vertex Raw(fvec2 pos, fvec4 color, fvec2 texcoord, fvec3 factors)
            {
                return Vertex(pos, iround(::clamp(color) * 255), texcoord, iround(::clamp(factors) * 255));
            }

            [[nodiscard]] static Vertex Color(fvec2 pos, fvec3 color, float alpha = 1, float beta = 1)
            {
                return Raw(pos, color.to_vec4(alpha), fvec2(0), fvec3(0, 0, beta));
            }
            [[nodiscard]] static Vertex Texture(fvec2 pos, fvec2 texcoord, float alpha = 1, float beta = 1)
            {
                return Raw(pos, fvec4(0), texcoord, fvec3(0, alpha, beta));
            }
            [[nodiscard]] static Vertex ColoredTexture(fvec2 pos, float mixing_factor, fvec3 color, fvec2 texcoord, float alpha = 1, float beta = 1) // mixing_factor: 0 - color, 1 - texture
            {
                return Raw(pos, color.to_vec4(0), texcoord, fvec3(mixing_factor, alpha, beta));
            }
        };

        using vertex_t = Vertex;
        using index_t = uint16_t;

      private:
        Geom::Queue<vertex_t, index_t, Geom::triangles> queue;
        Shader shader;

      public:
        Flat() {}

        Flat(size_t index_triangles, size_t vertices = 0)
            : queue(vertices ? vertices : index_triangles * 3, index_triangles)
        {}

        explicit operator bool() const
        {
            return bool(queue);
        }


        static void SetBlendingMode()
        {
            Graphics::Blending::FuncNormalPre();
        }


        Flat &operator<<(const Geom::Provider<Vertex, index_t> &provider)
        {
            DebugAssert("Attempt to use a null audio buffer.", *this);
            if (!*this)
                return *this;

            shader.Bind();
            queue.Insert(provider);
            return *this;
        }
    };
}
