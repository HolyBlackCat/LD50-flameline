#include "master.h"

#include "game/render.h"

#include <iomanip>
#include <iostream>
#include <vector>

#define main SDL_main

Program::Parachute error_parachute;
Interface::Window win("Alpha", vec(800, 600));
Graphics::DummyVertexArray dummy_vao;

struct A
{
    Reflect(A)
    (
        (fvec2)(pos),
        (fvec3)(color),
    )
};

Graphics::Shader sh("Main", Graphics::ShaderConfig::Core(150), Graphics::ShaderPreferences{}, Meta::tag<A>{}, Graphics::None, R"(
varying vec3 v_color;
void main()
{
    v_color = a_color;
    gl_Position = vec4(a_pos, 0, 1);
})",R"(
varying vec3 v_color;
void main()
{
    gl_FragColor = vec4(sin(pow(v_color.x*10. + sin(v_color.y*5.)*4., 2.))*0.5+0.5,
                        sin(pow(v_color.y*10. + sin(v_color.z*5.)*4., 2.))*0.5+0.5,
                        sin(pow(v_color.z*10. + sin(v_color.x*5.)*4., 2.))*0.5+0.5, 1);
})");

Graphics::RenderQueue<A, 3> queue(20);

int main(int, char**)
{
    sh.Bind();

    while (1)
    {
        win.ProcessEvents();

        if (win.Resized())
        {
            std::cout << "Resized\n";
            glViewport(0, 0, win.Size().x, win.Size().y);
        }
        if (win.ExitRequested())
            return 0;

        Graphics::Clear();
        queue.Add({fvec2(-0.5, 0.5), fvec3(1,0,0)},
                  {fvec2(   0,-0.5), fvec3(0,0,1)},
                  {fvec2( 0.5, 0.5), fvec3(0,1,0)},
                  {fvec2(   0, 0.75), fvec3(1,0,1)}
                  );
        queue.Flush();
        Graphics::CheckErrors();

        win.SwapBuffers();
    }

    return 0;
}
