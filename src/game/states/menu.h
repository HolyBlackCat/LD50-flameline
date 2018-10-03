#pragma once

#include "game/state.h"
#include "game/states/menu/background.h"

namespace States
{
    struct Menu : State
    {
        Details::Menu::Background bg;

        Menu() {}

        void Tick() override
        {
            bg.Tick();
        }
        void Render() const override
        {
            bg.Render();
        }
    };
}
