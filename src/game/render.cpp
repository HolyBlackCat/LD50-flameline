#include "render.h"

#include <algorithm>

#include "graphics/complete.h"
#include "reflection/complete.h"

struct Render::Data
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
    gl_FragColor = vec4(mix(v_color.rgb, tex_color.rgb, v_factors.x),
                        mix(v_color.a  , tex_color.a  , v_factors.y));
    vec4 result = u_color_matrix * vec4(gl_FragColor.rgb, 1);
    gl_FragColor.a *= result.a;
    gl_FragColor.rgb = result.rgb * gl_FragColor.a;
    gl_FragColor.a *= v_factors.z;
})";

    Graphics::RenderQueue<Attribs, 3> queue = nullptr; // Note that the queue has to be the first field.
    Uniforms uni;
    Graphics::Shader shader = nullptr;

    Data(int queue_size, const Graphics::ShaderConfig &config) : queue(queue_size), shader("Main", config, Graphics::ShaderPreferences{}, Meta::tag<Attribs>{}, uni, vertex_source, fragment_source) {}
};

void *Render::GetRenderQueuePtr()
{
    return &data->queue;
}

Render::Render(decltype(nullptr)) {}

Render::Render(int queue_size, const Graphics::ShaderConfig &config)
{
    data = std::make_unique<Data>(queue_size, config);
    SetMatrix(fmat4());
    SetColorMatrix(fmat4());
}

Render::~Render() = default;

Render::operator bool() const
{
    return bool(data->shader);
}

void Render::BindShader() const
{
    data->shader.Bind();
}

void Render::Finish()
{
    data->queue.Flush();
}

void Render::SetTextureUnit(const Graphics::TexUnit &unit)
{
    Finish();
    data->uni.texture = unit;
}

void Render::SetTextureSize(ivec2 size)
{
    Finish();
    data->uni.tex_size = size;
}

void Render::SetTexture(const Graphics::Texture &tex)
{
    SetTextureUnit(tex);
    SetTextureSize(tex.Size());
}

void Render::SetMatrix(const fmat4 &m)
{
    Finish();
    data->uni.matrix = m;
}

void Render::SetColorMatrix(const fmat4 &m)
{
    Finish();
    data->uni.color_matrix = m;
}

Render::Quad_t::~Quad_t()
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

    Render::Data::Attribs out[4];

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

    ((decltype(Render::Data::queue) *)queue)->Add(out[0], out[1], out[2], out[3]);
}

Render::Triangle_t::~Triangle_t()
{
    if (!queue)
        return;

    DebugAssert("2D poly renderer: Triangle with no texture nor color specified.", data.has_texture || data.has_color);
    DebugAssert("2D poly renderer: Triangle with texture and color, but without a mixing factor.", (data.has_texture && data.has_color) == data.has_tex_color_fac);

    Render::Data::Attribs out[3];

    if (data.has_texture)
    {
        for (int i = 0; i < 3; i++)
        {
            out[i].color = data.colors[i].to_vec4(0);
            out[i].factors.x = data.tex_color_factors[i];
            out[i].factors.y = data.alpha[i];
        }
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            out[i].color = data.colors[i].to_vec4(data.alpha[i]);
            out[i].factors.x = out[i].factors.y = 0;
        }
    }

    for (int i = 0; i < 3; i++)
        out[i].factors.z = data.beta[i];

    for (int i = 0; i < 3; i++)
    {
        out[i].pos = data.pos[i];
        out[i].texcoord = data.tex_pos[i];
    }

    if (data.has_matrix)
    {
        for (auto &it : out)
            it.pos = (data.matrix * it.pos.to_vec3(1)).to_vec2();
    }

    ((decltype(Render::Data::queue) *)queue)->Add(out[0], out[1], out[2]);
}

Render::Text_t::~Text_t()
{
    if (!renderer)
        return;

    Graphics::Text::Stats stats = data.text.ComputeStats();

    ivec2 align_box(data.has_box_alignment ? data.align_box_x : data.align.x, data.align.y);

    fvec2 pos = data.pos;
    pos -= stats.size * (1 + align_box) / 2;
    pos.x += stats.size.x * (1 + data.align.x) / 2; // Note that we don't change vertical position here.

    float start_x = pos.x;

    for (size_t line_index = 0; line_index < data.text.lines.size(); line_index++)
    {
        const Graphics::Text::Line &line = data.text.lines[line_index];
        const Graphics::Text::Stats::Line &line_stats = stats.lines[line_index];

        pos.x = start_x - line_stats.width * (1 + data.align.x) / 2;
        pos.y += line_stats.ascent;

        for (const Graphics::Text::Symbol &symbol : line.symbols)
        {
            renderer->fquad(pos + symbol.offset, symbol.size).tex(symbol.texture_pos).color(data.color).mix(0).alpha(data.alpha).beta(data.beta);
            pos.x += symbol.advance + symbol.kerning;
        }

        pos.y += line_stats.descent + line_stats.gap;
    }
}
