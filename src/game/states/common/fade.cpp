#include "program/parachute.h"

#include "game/states/common/fade.h"
#include "game/states/common/config.h"
#include "game/meta.h"

namespace States::Details::Common
{
    void Fade::Tick()
    {
        clamp_var(state -= Cfg::common->fade_step);
    }

    void Fade::Render() const
    {
        render.iquad(-screen_size/2, screen_size).color(color).alpha(state);
    }
}

