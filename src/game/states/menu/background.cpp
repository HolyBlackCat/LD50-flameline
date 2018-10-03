#include "program/parachute.h"

#include <utility>
#include <vector>

#include "game/states/menu/background.h"
#include "game/meta.h"

#include "graphics/clear.h"

namespace States::Details::Menu
{
    ReflectStruct(BackgroundConfig, (
        (float)(corner_offset)(=20),
        (float)(corner_size)(=20),
        (float)(corner_speed)(=0.01),
        (fvec3)(corner_color)(=fvec3(0,0,0)),
    ))

    static Config<BackgroundConfig> cfg("assets/config/menu.background.refl");

    void Background::Tick()
    {
        time++;
    }

    void Background::Render() const
    {
        Graphics::Clear();

        for (int y = -1; y <= 1; y += 2)
        for (int x = -1; x <= 1; x += 2)
        {
            render.iquad(iround(screen_size/2 - cfg->corner_offset) * ivec2(x,y), ivec2(cfg->corner_size)).color(cfg->corner_color).center().rotate(time * cfg->corner_speed * x * y);
        }
    }
}
