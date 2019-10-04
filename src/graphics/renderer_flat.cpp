#include "renderer_flat.h"

namespace Graphics::Renderers
{
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
    gl_FragColor.rgb *= gl_FragColor.a;
    gl_FragColor.a *= v_factors.z;
})";

    Flat::Flat(const Graphics::ShaderConfig &shader_config,size_t index_triangles, size_t vertices)
        : queue(vertices ? vertices : index_triangles * 3, index_triangles),
        shader("Flat renderer", shader_config, {}, Meta::tag<Vertex>{}, uniforms, vertex_source, fragment_source)
    {
        uniforms.matrix = fmat4();
    }
}
