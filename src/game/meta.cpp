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
#include "utils/json.h"
#include "utils/macro.h"
#include "utils/mat.h"
#include "utils/memory_file.h"
#include "utils/meta.h"
#include "utils/metronome.h"
#include "utils/random.h"
#include "utils/resource_allocator.h"
#include "utils/robust_compare.h"
#include "utils/strings.h"
#include "utils/tasks.h"

#include "game/adaptive_viewport.h"
#include "game/map.h"
#include "game/render.h"
#include "game/state.h"
#include "game/states/world.h"

#define main SDL_main

extern const ivec2 screen_size = ivec2(1920,1080)/4;

static Program::Parachute error_parachute;

Interface::Window win("Gamma", screen_size*2);

static Graphics::DummyVertexArray dummy_vao;

Random random;

Render render(1000, Graphics::ShaderConfig::Core());

static Graphics::Texture tex = Graphics::Texture().SetData(Graphics::Image("assets/texture.png")).Interpolation(Graphics::nearest).Wrap(Graphics::clamp);

AdaptiveViewport viewport(Graphics::ShaderConfig::Core(), screen_size);

Metronome metronome;

Input::Mouse mouse;

int main(int, char**)
{
    Graphics::Blending::Enable();
    Graphics::Blending::FuncNormalPre();

    render.SetTexture(tex);
    viewport.Update();

    render.SetMatrix(viewport.GetDetails().MatrixCentered());
    mouse.SetMatrix(viewport.GetDetails().MouseMatrixCentered());

    Clock::DeltaTimer delta_timer;

    DynStorage<States::State> state = decltype(state)::make<States::World>();

    while (1)
    {
        uint64_t delta = delta_timer();
        while (metronome.Tick(delta))
        {
            win.ProcessEvents();

            if (win.Resized())
            {
                std::cout << "Resized\n";
                viewport.Update();
                mouse.SetMatrix(viewport.GetDetails().MouseMatrixCentered());
            }
            if (win.ExitRequested())
                return 0;

            state->Tick();
        }

        viewport.BeginFrame();
        Graphics::Clear();
        render.BindShader();
        state->Render();
        render.Finish();
        viewport.FinishFrame();
        Graphics::CheckErrors();

        win.SwapBuffers();
    }

    return 0;
}
