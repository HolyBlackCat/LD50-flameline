#pragma once

#include <type_traits>
#include <utility>

#include "graphics/complete.h"
#include "reflection/complete.h"
#include "utils/mat.h"

namespace Graphics
{
    struct ShaderConfig;
}

class Render
{
    ReflectStruct(Attribs, (
        (fvec2)(pos),
        (fvec4)(color),
        (fvec2)(texcoord),
        (fvec3)(factors),
    ))

    ReflectStruct(Uniforms, (
        (Graphics::VertUniform<fmat4>)(matrix),
        (Graphics::VertUniform<fvec2>)(tex_size),
        (Graphics::FragUniform<Graphics::TexUnit>)(texture),
        (Graphics::FragUniform<fmat4>)(color_matrix),
    ))

    static constexpr const char *vertex_source = R"(
varying vec4 v_color;
varying vec2 v_texcoord;
varying vec3 v_factors;
void main()
{
    gl_Position = u_matrix * vec4(a_pos, 0, 1);
    v_color     = a_color;
    v_texcoord  = a_texcoord / u_tex_size;
    v_factors   = a_factors;
})";

    static constexpr const char *fragment_source = R"(
varying vec4 v_color;
varying vec2 v_texcoord;
varying vec3 v_factors;
void main()
{
    vec4 tex_color = texture2D(u_texture, v_texcoord);
    gl_FragColor = vec4(mix(v_color.rgb, tex_color.rgb, v_factors.x)
                        mix(v_color.a  , tex_color.a  , v_factors.y));
    vec4 result = u_color_matrix * vec4(gl_FragColor.rgb, 1);
    gl_FragColor.a *= result.a;
    gl_FragColor.rgb = result.rgb * gl_FragColor.a;
    gl_FragColor.a *= v_factors.z;
})";

    Graphics::RenderQueue<Attribs, 3> queue = Graphics::RenderQueue<Attribs, 3>(nullptr);
    Uniforms uni;
    Graphics::Shader shader = Graphics::Shader(nullptr);

  public:
    explicit Render(decltype(nullptr)) {}
    Render(int queue_size, const Graphics::ShaderConfig &config) : queue(queue_size), shader("Main", config, Graphics::ShaderPreferences{}, Meta::tag<Attribs>{}, uni, vertex_source, fragment_source)
    {
        SetMatrix(fmat4());
        SetColorMatrix(fmat4());
    }

    explicit operator bool() const
    {
        return bool(shader);
    }

    void BindShader() const
    {
        shader.Bind();
    }

    void Finish()
    {
        queue.Flush();
    }

    void SetTexture(const Graphics::TexUnit &unit, ivec2 size)
    {
        Finish();
        uni.texture = unit;
        uni.tex_size = size;
    }
    void SetTexture(Graphics::TexUnit &&) = delete;

    void SetMatrix(const fmat4 &m)
    {
        Finish();
        uni.matrix = m;
    }

    void SetColorMatrix(const fmat4 &m)
    {
        Finish();
        uni.color_matrix = m;
    }

    class Quad_t
    {
        using ref = Quad_t &&;

        Graphics::RenderQueue<Attribs, 3> *queue; // The constructor sets this.

        struct Data
        {
            fvec2 pos, size; // The constructor set these.

            bool has_texture = 0;
            fvec2 tex_pos = fvec2(0), tex_size = fvec2(0);

            bool has_center = 0;
            fvec2 center = fvec2(0);
            bool center_pos_tex = 0;

            bool has_matrix = 0;
            fmat3 matrix = fmat3();

            bool has_color = 0;
            fvec3 colors[4] {};

            bool has_tex_color_fac = 0;
            float tex_color_factors[4] = {1,1,1,1};

            float alpha[4] = {1,1,1,1};
            float beta[4] = {1,1,1,1};

            bool abs_pos = 0;
            bool abs_tex_pos = 0;

            bool flip_x = 0, flip_y = 0;
        };
        Data data;

      public:
        Quad_t(decltype(queue) queue, fvec2 pos, fvec2 size) : queue(queue)
        {
            data.pos = pos;
            data.size = size;
        }
        Quad_t(Quad_t &&other) noexcept : queue(std::exchange(other.queue, {})), data(std::move(other.data)) {}
        Quad_t &operator=(Quad_t other)
        {
            std::swap(queue, other.queue);
            std::swap(data, other.data);
            return *this;
        }

        ~Quad_t()
        {
           if (!queue)
                return;

            DebugAssert("2D poly renderer: Quad with no texture nor color specified.", data.has_texture || data.has_color);
            DebugAssert("2D poly renderer: Quad with absolute corner coodinates with a center specified.", data.abs_pos + data.has_center < 2);
            DebugAssert("2D poly renderer: Quad with absolute texture coordinates mode but no texture coordinates specified.", data.abs_tex_pos <= data.has_texture);
            DebugAssert("2D poly renderer: Quad with texture and color, but without a mixing factor.", (data.has_texture && data.has_color) == data.has_tex_color_fac);
            DebugAssert("2D poly renderer: Quad with a matrix but without a center specified.", data.has_matrix <= data.has_center);

            if (data.abs_pos)
                data.size -= data.pos;
            if (data.abs_tex_pos)
                data.tex_size -= data.tex_pos;

            Attribs out[4];

            if (data.has_texture)
            {
                for (int i = 0; i < 4; i++)
                {
                    out[i].color = data.colors[i].to_vec4(0);
                    out[i].factors.x = data.tex_color_factors[i];
                    out[i].factors.y = data.alpha[i];
                }

                if (data.center_pos_tex)
                {
                    if (data.tex_size.x)
                        data.center.x *= data.size.x / data.tex_size.x;
                    if (data.tex_size.y)
                        data.center.y *= data.size.y / data.tex_size.y;
                }
            }
            else
            {
                for (int i = 0; i < 4; i++)
                {
                    out[i].color = data.colors[i].to_vec4(data.alpha[i]);
                    out[i].factors.x = out[i].factors.y = 0;
                }
            }

            for (int i = 0; i < 4; i++)
                out[i].factors.z = data.beta[i];

            if (data.flip_x)
            {
                data.tex_pos.x += data.tex_size.x;
                data.tex_size.x = -data.tex_size.x;
                if (data.has_center)
                    data.center.x = data.size.x - data.center.x;
            }
            if (data.flip_y)
            {
                data.tex_pos.y += data.tex_size.y;
                data.tex_size.y = -data.tex_size.y;
                if (data.has_center)
                    data.center.y = data.size.y - data.center.y;
            }

            out[0].pos = -data.center;
            out[2].pos = data.size - data.center;
            out[1].pos = fvec2(out[2].pos.x, out[0].pos.y);
            out[3].pos = fvec2(out[0].pos.x, out[2].pos.y);

            if (data.has_matrix)
            {
                for (auto &it : out)
                    it.pos = data.pos + (data.matrix * it.pos.to_vec3(1)).to_vec2();
            }
            else
            {
                for (auto &it : out)
                    it.pos += data.pos;
            }

            out[0].texcoord = data.tex_pos;
            out[2].texcoord = data.tex_pos + data.tex_size;
            out[1].texcoord = {out[2].texcoord.x, out[0].texcoord.y};
            out[3].texcoord = {out[0].texcoord.x, out[2].texcoord.y};

            queue->Add(out[0], out[1], out[2], out[3]);
        }

        ref tex(fvec2 pos, fvec2 size)
        {
            DebugAssert("2D poly renderer: Quad_t texture specified twice.", !data.has_texture);
            data.has_texture = 1;

            data.tex_pos = pos;
            data.tex_size = size;
            return (ref)*this;
        }
        ref tex(fvec2 pos)
        {
            tex(pos, data.size);
            return (ref)*this;
        }
        ref center(fvec2 c)
        {
            DebugAssert("2D poly renderer: Quad_t center specified twice.", !data.has_center);
            data.has_center = 1;

            data.center = c;
            data.center_pos_tex = 1;
            return (ref)*this;
        }
        ref pixel_center(fvec2 c) // Same as `center()`, but the coordinates are always measured in pixels instead of texels even if a texture is specified.
        {
            DebugAssert("2D poly renderer: Quad_t center specified twice.", !data.has_center);
            data.has_center = 1;

            data.center = c;
            data.center_pos_tex = 0;
            return (ref)*this;
        }
        ref center()
        {
            pixel_center(data.size / 2);
            return (ref)*this;
        }
        ref matrix(fmat3 m) // This can be called multiple times, resulting in multiplying matrices in the order they were passed.
        {
            if (data.has_matrix)
            {
                data.matrix = data.matrix * m;
            }
            else
            {
                data.has_matrix = 1;
                data.matrix = m;
            }
            return (ref)*this;
        }
        ref matrix(fmat2 m)
        {
            matrix(m.to_mat3());
            return (ref)*this;
        }
        ref rotate(float a) // Uses `matrix()`.
        {
            matrix(fmat3::rotate(a));
            return (ref)*this;
        }
        ref translate(fvec2 v) // Uses a matrix.
        {
            matrix(fmat3::translate(v));
            return (ref)*this;
        }
        ref scale(fvec2 s) // Uses a matrix.
        {
            matrix(fmat3::scale(s));
            return (ref)*this;
        }
        ref scale(float s) // Uses a matrix.
        {
            scale(fvec2(s));
            return (ref)*this;
        }
        ref color(fvec3 c)
        {
            DebugAssert("2D poly renderer: Quad_t color specified twice.", !data.has_color);
            data.has_color = 1;

            for (auto &it : data.colors)
                it = c;
            return (ref)*this;
        }
        ref color(fvec3 a, fvec3 b, fvec3 c, fvec3 d)
        {
            DebugAssert("2D poly renderer: Quad_t color specified twice.", !data.has_color);
            data.has_color = 1;

            data.colors[0] = a;
            data.colors[1] = b;
            data.colors[2] = c;
            data.colors[3] = d;
            return (ref)*this;
        }
        ref mix(float x) // 0 - fill with color, 1 - use texture
        {
            DebugAssert("2D poly renderer: Quad_t texture/color factor specified twice.", !data.has_tex_color_fac);
            data.has_tex_color_fac = 1;

            for (auto &it : data.tex_color_factors)
                it = x;
            return (ref)*this;
        }
        ref mix(float a, float b, float c, float d)
        {
            DebugAssert("2D poly renderer: Quad_t texture/color factor specified twice.", !data.has_tex_color_fac);
            data.has_tex_color_fac = 1;

            data.tex_color_factors[0] = a;
            data.tex_color_factors[1] = b;
            data.tex_color_factors[2] = c;
            data.tex_color_factors[3] = d;
            return (ref)*this;
        }
        ref alpha(float a)
        {
            for (auto &it : data.alpha)
                it = a;
            return (ref)*this;
        }
        ref alpha(float a, float b, float c, float d)
        {
            data.alpha[0] = a;
            data.alpha[1] = b;
            data.alpha[2] = c;
            data.alpha[3] = d;
            return (ref)*this;
        }
        ref beta(float a) // 1 - normal blending, 0 - additive blending
        {
            for (auto &it : data.beta)
                it = a;
            return (ref)*this;
        }
        ref beta(float a, float b, float c, float d)
        {
            data.beta[0] = a;
            data.beta[1] = b;
            data.beta[2] = c;
            data.beta[3] = d;
            return (ref)*this;
        }
        ref absolute(bool x = 1) // Interpret size as a position of the second corner
        {
            data.abs_pos = x;
            return (ref)*this;
        }
        ref absolute_tex(bool x = 1) // Interpret texture size as a position of the second corner
        {
            data.abs_tex_pos = x;
            return (ref)*this;
        }
        ref flip_x(bool f = 1) // Flips texture horizontally if it was specified. Updates the center accordingly if it was specified.
        {
            data.flip_x = f;
            return (ref)*this;
        }
        ref flip_y(bool f = 1) // Flips texture vertically if it was specified. Updates the center accordingly if it was specified.
        {
            data.flip_y = f;
            return (ref)*this;
        }
    };
};
