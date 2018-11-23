#include "program/parachute.h"

#include <iomanip>
#include <iostream>
#include <vector>

#include "graphics/complete.h"
#include "input/mouse.h"
#include "interface/window.h"
#include "utils/adjust.h"
#include "utils/mat.h"
#include "utils/metronome.h"
#include "utils/strings.h"

#include "game/adaptive_viewport.h"
#include "game/controls.h"
#include "game/meta.h"
#include "game/render.h"
#include "game/state.h"
#include "game/texture_atlas.h"
#include "game/states/world.h"

#define main SDL_main

extern const ivec2 screen_size = ivec2(480,270);
static const Graphics::ShaderConfig shader_config;

Interface::Window win("Gamma", screen_size*2, Interface::windowed, ADJUST(Interface::WindowSettings{}, min_size = screen_size));

static Graphics::DummyVertexArray dummy_vao;

// The `<>` should only be needed in `extern` declarations (since omitting it apparently requires class template argument deduction, which doesn't work at all with `extern` declarations).
// Due to a GCC bug, the presence of `extern` declaration makes `<>` necessary here as well.
Random<> random;

Render render(1000, shader_config);

const TextureAtlas &Atlas()
{
    static TextureAtlas atlas(ivec2(2048), "assets/images", "assets/atlas.png", "assets/atlas.refl");
    return atlas;
}

static Graphics::Texture tex = Graphics::Texture().SetData(Atlas().GetImage()).Interpolation(Graphics::nearest).Wrap(Graphics::clamp);

AdaptiveViewport viewport(shader_config, screen_size);

Metronome metronome;

DynStorage<States::State> game_state = nullptr;


int main(int, char**)
{
    Atlas(); // Generate the atlas if it wasn't done before.

    Graphics::SetClearColor(fvec3(0));

    Graphics::Blending::Enable();
    Graphics::Blending::FuncNormalPre();

    render.SetTexture(tex);
    viewport.Update();

    render.SetMatrix(viewport.GetDetails().MatrixCentered());
    controls.mouse.SetMatrix(viewport.GetDetails().MouseMatrixCentered());

    Clock::DeltaTimer delta_timer;

    game_state = game_state.make<States::World>();

    while (1)
    {
        uint64_t delta = delta_timer();
        while (metronome.Tick(delta))
        {
            win.ProcessEvents();

            if (win.Resized())
            {
                viewport.Update();
                controls.mouse.SetMatrix(viewport.GetDetails().MouseMatrixCentered());
            }
            if (win.ExitRequested())
                return 0;

            game_state->Tick();
        }

        game_state->Render();

        win.SwapBuffers();
    }

    return 0;
}
