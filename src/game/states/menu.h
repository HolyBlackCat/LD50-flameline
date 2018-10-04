#pragma once

#include "game/state.h"
#include "game/states/menu/background.h"
#include "game/states/menu/buttons.h"
#include "game/states/common/fade.h"

namespace States
{
    struct Menu : State
    {
        Details::Menu::Background bg;
        Details::Menu::Buttons buttons;
        Details::Common::Fade fade = decltype(fade)(fvec3(0));

        Menu() {}

        void Tick() override
        {
            buttons.Tick(*this);
            fade.Tick();
        }
        void Render() const override
        {
            bg.Render();
            buttons.Render();
            fade.Render();
        }
    };
}
