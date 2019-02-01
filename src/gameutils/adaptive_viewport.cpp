#include "adaptive_viewport.h"

#include "graphics/clear.h"
#include "graphics/framebuffer.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/vertex_buffer.h"
#include "graphics/viewport.h"
#include "interface/window.h"
#include "reflection/complete.h"

struct AdaptiveViewport::Data
{
    ReflectStruct(ShaderAttribs,(
        (fvec2)(pos),
    ))
    ReflectStruct(ShaderUniforms,(
        (Graphics::FragUniform<Graphics::TexUnit>)(texture),
    ))

    static constexpr const char *shader_vert_src = R"(
varying vec2 v_texcoord;
void main()
{
    v_texcoord = a_pos * 0.5 + 0.5;
    gl_Position = vec4(a_pos, 0, 1);
})";
    static constexpr const char *shader_frag_src = R"(
varying vec2 v_texcoord;
void main()
{
    gl_FragColor = texture2D(u_texture, v_texcoord);
})";

    Details details;
    Graphics::Shader shader = nullptr;
    ShaderUniforms shader_uni;
    Graphics::TexObject fbuf_tex = nullptr, fbuf_tex_intermediate = nullptr;
    Graphics::TexUnit tex_unit = nullptr;
    Graphics::FrameBuffer fbuf = nullptr, fbuf_intermediate = nullptr;
    Graphics::VertexBuffer<ShaderAttribs> vertex_buf = nullptr;
};

AdaptiveViewport::AdaptiveViewport(decltype(nullptr)) {}

AdaptiveViewport::AdaptiveViewport(const Graphics::ShaderConfig &shader_config) : data(std::make_unique<Data>())
{
    data->shader = Graphics::Shader("Adaptive viewport identity shader", shader_config, Graphics::ShaderPreferences{}, Meta::tag<Data::ShaderAttribs>{}, data->shader_uni, Data::shader_vert_src, Data::shader_frag_src);
    data->fbuf_tex = Graphics::TexObject();
    data->fbuf_tex_intermediate = Graphics::TexObject();
    data->tex_unit = Graphics::TexUnit();
    data->shader_uni.texture = data->tex_unit;
    data->tex_unit.Attach(data->fbuf_tex).Wrap(Graphics::clamp).Interpolation(Graphics::nearest);
    data->tex_unit.Attach(data->fbuf_tex_intermediate).Wrap(Graphics::clamp).Interpolation(Graphics::linear);
    data->fbuf = Graphics::FrameBuffer().Attach(data->fbuf_tex);
    data->fbuf_intermediate = Graphics::FrameBuffer().Attach(data->fbuf_tex_intermediate);

    constexpr float margin = 0.01;
    constexpr int vertex_count = 3;
    Data::ShaderAttribs vertex_data[vertex_count]
    {
        {ivec2(-1-margin, -1-margin)},
        {ivec2(-1-margin,  3+margin)},
        {ivec2( 3+margin, -1-margin)},
    };
    data->vertex_buf = Graphics::VertexBuffer<Data::ShaderAttribs>(vertex_count, vertex_data);
}
AdaptiveViewport::AdaptiveViewport(const Graphics::ShaderConfig &shader_config, ivec2 new_size) : AdaptiveViewport(shader_config)
{
    SetSize(new_size);
}

AdaptiveViewport::~AdaptiveViewport() {}

void AdaptiveViewport::SetSize(ivec2 new_size)
{
    data->details.SetSize(new_size);
    data->tex_unit.Attach(data->fbuf_tex).SetData(new_size);
}

void AdaptiveViewport::Update(ivec2 new_target_size)
{
    data->details.Update(new_target_size);
    data->tex_unit.Attach(data->fbuf_tex_intermediate).SetData(data->details.IntermediateSize());
}

void AdaptiveViewport::Update()
{
    Update(Interface::Window::Get().Size());
}

void AdaptiveViewport::BeginFrame()
{
    data->fbuf.Bind();
    Graphics::Viewport(data->details.Size());
}

void AdaptiveViewport::FinishFrame(const Graphics::FrameBuffer *fbuf)
{
    data->shader.Bind();

    data->fbuf_intermediate.Bind();
    Graphics::Viewport(data->details.IntermediateSize());
    data->tex_unit.Attach(data->fbuf_tex);
    data->vertex_buf.Draw(Graphics::triangles);

    if (fbuf)
        fbuf->Bind();
    else
        Graphics::FrameBuffer::BindDefault();
    Graphics::Viewport(data->details.ViewportPos(), data->details.ViewportSize());
    Graphics::Clear();
    data->tex_unit.Attach(data->fbuf_tex_intermediate);
    data->vertex_buf.Draw(Graphics::triangles);
}

const AdaptiveViewport::Details &AdaptiveViewport::GetDetails() const
{
    return data->details;
}
