#include "program/parachute.h"

#include <utility>
#include <vector>

#include "game/states/menu/background.h"
#include "game/meta.h"

#include "graphics/clear.h"

namespace States::Details::Menu
{
    static const auto
        img_background = Atlas().Get("backgrounds/menu.png");

    void Background::Render() const
    {
        render.iquad(ivec2(0), img_background.size).tex(img_background.pos).center();
    }
}
