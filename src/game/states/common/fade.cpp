#include "program/parachute.h"

#include <utility>

#include "game/states/common/fade.h"
#include "game/states/common/config.h"
#include "game/meta.h"
#include "program/exit.h"

namespace States::Details::Common
{
    static auto &cfg = *Cfg::Common();

    void Fade::Tick()
    {
        if (ChangingState())
        {
            alpha += cfg.fade_step;
            if (alpha > 1)
            {
                if (next_state)
                    game_state = std::move(next_state);
                else
                    Program::Exit();
            }
        }
        else
        {
            alpha -= cfg.fade_step;
            if (alpha < 0)
                alpha = 0;
        }
    }

    void Fade::Render() const
    {
        render.iquad(-screen_size/2, screen_size).color(color).alpha(alpha);
    }

    void Fade::SetNextState(DynStorage<State> state)
    {
        have_next_state = 1;
        next_state = std::move(state);
    }

    bool Fade::ChangingState() const
    {
        return have_next_state;
    }
}

