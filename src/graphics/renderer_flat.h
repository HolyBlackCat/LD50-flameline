#pragma once

#include "graphics/blending.h"
#include "graphics/geometry.h"
#include "graphics/shader.h"
#include "program/errors.h"
#include "reflection/complete.h"
#include "utils/mat.h"

namespace Graphics::Renderers
{


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
