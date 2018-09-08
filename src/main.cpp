#include <iomanip>
#include <iostream>
#include <vector>

#include "graphics/complete.h"
#include "input/complete.h"
#include "interface/messagebox.h"
#include "interface/window.h"
#include "program/errors.h"
#include "program/exit.h"
#include "program/parachute.h"
#include "reflection/complete.h"
#include "utils/adjust.h"
#include "utils/archive.h"
#include "utils/clock.h"
#include "utils/dynamic_storage.h"
#include "utils/finally.h"
#include "utils/macro.h"
#include "utils/mat.h"
#include "utils/memory_file.h"
#include "utils/meta.h"
#include "utils/metronome.h"
#include "utils/resource_allocator.h"
#include "utils/strings.h"
#include "utils/tasks.h"

#include "game/render.h"

#define main SDL_main

Program::Parachute error_parachute;
Interface::Window win("Alpha", ivec2(800, 600));
Graphics::DummyVertexArray dummy_vao;

Render render(1000, Graphics::ShaderConfig::Core());
Graphics::Texture tex = Graphics::Texture().SetData(Graphics::Image("texture.png")).Interpolation(Graphics::nearest).Wrap(Graphics::clamp);

Input::Mouse mouse(fmat3::translate(-fvec2(400,300)));

int main(int, char**)
{
    render.SetTexture(tex);
    render.SetMatrix(fmat4::ortho(ivec2(-400,300), ivec2(400,-300), -1, 1));
    render.BindShader();

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
        render.fquad(mouse.pos(), fvec2(32)).center().color(fvec3(1,0.5,0));
        render.fquad(mouse.pos().add_y(32), fvec2(16)).center().tex(fvec2(16,0));
        render.Finish();
        Graphics::CheckErrors();

        win.SwapBuffers();
    }

    return 0;
}
