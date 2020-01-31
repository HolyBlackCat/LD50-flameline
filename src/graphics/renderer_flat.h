#pragma once

#include <algorithm>

#include "graphics/blending.h"
#include "graphics/geometry.h"
#include "graphics/shader.h"
#include "graphics/texture_atlas.h"
#include "graphics/texture.h"
#include "graphics/transformations.h"
#include "meta/misc.h"
#include "program/errors.h"
#include "reflection/full.h"
#include "utils/mat.h"

namespace Graphics::Renderers
{
    class Flat : public TransformFuncs<fvec2>
    {
      public:
        struct Vertex
        {
            // factors.x: 0 - use color, 255 - use texture
            // factors.y: 0 - use color.a as alpha, 255 - use alpha from texture
            // factors.z: 0 - additive blending, 255 - regular blending

            // Note that fields are declared in a weird order to improve packing.
            REFL_MEMBERS
            (
                REFL_DECL(fvec2 REFL_INIT{}) pos
                REFL_DECL(fvec2 REFL_INIT{}) texcoord
                REFL_DECL(fvec4 REFL_INIT{}) color
                REFL_DECL(fvec3 REFL_INIT{}) factors
            )

            constexpr auto &GetTransformableVertexPosition()
            {
                return pos;
            }

            Vertex() {}

            Vertex(fvec2 pos, fvec4 color, fvec2 texcoord, fvec3 factors)
                : pos(pos), texcoord(texcoord), color{color}, factors{factors}
            {}

            [[nodiscard]] static Vertex Raw(fvec2 pos, fvec4 color, fvec2 texcoord, fvec3 factors)
            {
                return Vertex(pos, color, texcoord, factors);
            }

            [[nodiscard]] static Vertex Color(fvec2 pos, fvec3 color, float alpha = 1, float beta = 1)
            {
                return Raw(pos, color.to_vec4(alpha), fvec2(0), fvec3(0, 0, beta));
            }
            [[nodiscard]] static Vertex Texture(fvec2 pos, fvec2 texcoord, float alpha = 1, float beta = 1)
            {
                return Raw(pos, fvec4(0), texcoord, fvec3(1, alpha, beta));
            }
            [[nodiscard]] static Vertex ColoredTexture(fvec2 pos, float mixing_factor, fvec3 color, fvec2 texcoord, float alpha = 1, float beta = 1) // mixing_factor: 0 - color, 1 - texture
            {
                return Raw(pos, color.to_vec4(0), texcoord, fvec3(mixing_factor, alpha, beta));
            }
        };

        using Index = uint16_t;

        template <typename VC> using DataFlat = Geom::DataFlat<VC>;
        template <typename VC, typename IC> using Data = Geom::Data<VC, IC>;

        using RefFlat = Geom::RefFlat<Vertex>;
        using Ref = Geom::Ref<Vertex, Index>;

      private:
        Geom::Queue<Vertex, Index, Geom::triangles> queue;

        REFL_SIMPLE_STRUCT( Uniforms
            REFL_DECL(Uniform<fmat4> REFL_ATTR Vert) matrix
            REFL_DECL(Uniform<fvec2> REFL_ATTR Vert) tex_size
            REFL_DECL(Uniform<TexUnit> REFL_ATTR Frag) texture
        )

        Uniforms uniforms; // Note that this has to be created before `shader`.
        Shader shader;

      public:
        Flat() {}

        Flat(const Graphics::ShaderConfig &shader_config, size_t index_triangles, size_t vertices = 0);

        explicit operator bool() const
        {
            return bool(queue);
        }

        static void SetBlendingMode()
        {
            Graphics::Blending::FuncNormalPre();
        }

        void Flush()
        {
            queue.Flush();
        }

        void SetMatrix(fmat4 new_matrix)
        {
            DebugAssert("Attempt to use a null renderer.", *this);
            uniforms.matrix = new_matrix;
        }
        void ResetMatrix()
        {
            SetMatrix(fmat4());
        }

        void SetTexture(const Graphics::Texture &texture)
        {
            SetTextureUnit(texture);
            SetTextureSize(texture.Size());
        }

        void SetTextureUnit(const Graphics::TexUnit &new_unit)
        {
            uniforms.texture = new_unit;
        }
        void SetTextureSize(ivec2 new_size)
        {
            uniforms.tex_size = new_size;
        }

        Flat &operator<<(Geom::View<Vertex, Index> view)
        {
            DebugAssert("Attempt to use a null renderer.", *this);
            if (!*this)
                return *this;

            shader.Bind();
            queue.Insert(std::move(view));
            return *this;
        }

      private:
        struct GenericQuadData
        {
            struct Data
            {
                fvec2 pos{}; // Non-zero if `HasAlignment == true`.
                fvec2 size{}; // Always set.
                fvec3 color{}; // Set if `HasColor == true`.
                fvec2 tex_pos, tex_size{}; // Set if `HasTexture == true`.
                float color_tex_mixing = 0; // Set if `HasColor && HasTexture`
                float alpha = 1, beta = 1; // Non
            };

            Data data;
        };

        template <bool HasColor, bool HasTexture>
        struct GenericQuadDataFinished : GenericQuadData, Geom::Provider<Vertex, Index>
        {
            static_assert(HasColor || HasTexture);

          private:
            std::size_t VertexCount() const override final
            {
                return 4;
            }
            std::size_t IndexCount() const override final
            {
                return 6;
            }

            void GetVerticesLow(std::size_t begin, std::size_t end, Vertex *dest) const override final
            {
                static_assert(HasColor || HasTexture, "A color and/or a texture has to be specified.");

                for (std::size_t i = begin; i < end; i++)
                {
                    fvec2 vert_pos = data.pos;
                    if (i == 1 || i == 2)
                        vert_pos.x += data.size.x;
                    if (i == 2 || i == 3)
                        vert_pos.y += data.size.y;

                    if constexpr (HasTexture)
                    {
                        fvec2 vert_texcoord = data.tex_pos;
                        if (i == 1 || i == 2)
                            vert_texcoord.x += data.tex_size.x;
                        if (i == 2 || i == 3)
                            vert_texcoord.y += data.tex_size.y;

                        if constexpr (HasColor)
                            *dest = Vertex::ColoredTexture(vert_pos, data.color_tex_mixing, data.color, vert_texcoord, data.alpha, data.beta);
                        else
                            *dest = Vertex::Texture(vert_pos, vert_texcoord, data.alpha, data.beta);
                    }
                    else // HasColor && !HasTexture
                    {
                        *dest = Vertex::Color(vert_pos, data.color, data.alpha, data.beta);
                    }

                    dest++;
                }
            }

            const Index *IndexPointerIfAvailable() const override final
            {
                // 0----1
                // |  .'|
                // |.'  |
                // 3----2
                static constexpr Index indices[6] = {0,1,3, 2,3,1};
                return indices;
            }
        };

        template <bool HasTexture, bool HasColor, bool HasAlignment, bool HasOpacity>
        class GenericQuad
            : public std::conditional_t<HasColor || HasTexture, GenericQuadDataFinished<HasColor, HasTexture>, GenericQuadData>
        {
          public:
            GenericQuad() {}

            // Adjust alignment

            [[nodiscard]] auto CenterAbs(fvec2 center) const
            {
                static_assert(!HasAlignment, "Alignment already specfied.");

                GenericQuad<HasTexture, HasColor, true, HasOpacity> ret;
                ret.data = this->data;
                ret.data.pos = -center;
                return ret;
            }

            [[nodiscard]] auto CenterRel(fvec2 center_rel) const
            {
                return CenterAbs(center_rel * this->data.size);
            }

            [[nodiscard]] auto CenterTex(fvec2 center_tex) const
            {
                return CenterRel(center_tex / this->data.tex_size);
            }

            [[nodiscard]] auto Centered() const
            {
                return CenterRel(fvec2(0.5));
            }

            // Adjust color

            [[nodiscard]] auto Color(fvec3 color, float intensity) const
            {
                static_assert(!HasColor, "Color already specified.");
                static_assert(HasTexture, "Specifying color intensity only makes sense for textured quads.");

                GenericQuad<HasTexture, true, HasAlignment, HasOpacity> ret;
                ret.data = this->data;
                ret.data.color = color;
                ret.data.color_tex_mixing = 1 - intensity;
                return ret;
            }

            [[nodiscard]] auto Color(fvec3 color) const
            {
                static_assert(!HasColor, "Color already specified.");
                static_assert(!HasTexture, "Color intensity wasn't specified.");

                GenericQuad<HasTexture, true, HasAlignment, HasOpacity> ret;
                ret.data = this->data;
                ret.data.color = color;
                ret.data.color_tex_mixing = 0;
                return ret;
            }

            // Adjust opacity

            [[nodiscard]] auto Opacity(float alpha, float beta = 1) const
            {
                static_assert(!HasOpacity, "Color already specified.");

                GenericQuad<HasTexture, HasColor, HasAlignment, true> ret;
                ret.data = this->data;
                ret.data.alpha = alpha;
                ret.data.beta = beta;
                return ret;
            }
        };

      public:
        [[nodiscard]] static auto UntexturedQuad(fvec2 size)
        {
            GenericQuad<false, false, false, false> ret;
            ret.data.size = size;
            return ret;
        }

        [[nodiscard]] static auto TexturedQuad(fvec2 tex_pos, fvec2 tex_size, fvec2 size)
        {
            GenericQuad<true, false, false, false> ret;
            ret.data.size = size;
            ret.data.tex_pos = tex_pos;
            ret.data.tex_size = tex_size;
            return ret;
        }
        [[nodiscard]] static auto TexturedQuad(fvec2 tex_pos, fvec2 tex_size)
        {
            return TexturedQuad(tex_pos, tex_size, tex_size);
        }
        [[nodiscard]] static auto TexturedQuad(const TextureAtlas::Region &texture, fvec2 size)
        {
            return TexturedQuad(texture.pos, texture.size, size);
        }
        [[nodiscard]] static auto TexturedQuad(const TextureAtlas::Region &texture)
        {
            return TexturedQuad(texture.pos, texture.size);
        }
    };
}
