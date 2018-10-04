#include "program/parachute.h"

#include <utility>
#include <vector>

#include "game/states/menu.h"
#include "game/states/world.h"
#include "game/states/menu/buttons.h"
#include "game/states/menu/config.h"
#include "game/meta.h"
#include "game/controls.h"

namespace States::Details::Menu
{
    static auto &cfg = *Cfg::Menu();

    static const auto
        img_buttons = Atlas().Get("gui/menu.buttons.png");

    Buttons::Buttons() {}

    void Buttons::Tick(States::Menu &this_menu)
    {
        // Change button visuals
        for (int i = 0; i < button_count; i++)
        {
            auto &button = buttons[i];
            clamp_var(button.time += ((active_button == i)*2-1) * cfg.button_time_step);
        }

        // Stop here if we're exiting menu.
        if (this_menu.fade.ChangingState())
            return;

        // Select new button
        int change = controls.down.pressed() - controls.up.pressed();
        active_button = (active_button + button_count + change) % button_count;

        // Press a button
        if (controls.jump_ok.pressed())
        {
            switch (active_button)
            {
              case 0:
                this_menu.fade.SetNextState(decltype(game_state)::make<States::World>());
                break;
              case 1:
                this_menu.fade.SetNextState({});
                break;
            }
        }
    }

    void Buttons::Render() const
    {
        for (int i = 0; i < button_count; i++)
        {
            const auto &button = buttons[i];

            for (int j = 0; j < 2; j++)
            {
                fvec3 color;
                if (j)
                    color = button.time * cfg.button_color_active + (1-button.time) * cfg.button_color_main;
                else
                    color = cfg.button_color_shadow;

                render.iquad(cfg.button_offset + ivec2(-img_buttons.size.x/2, i * cfg.button_y_step) + (1-j), ivec2(img_buttons.size.x, cfg.button_y_size))
                      .tex(img_buttons.pos.add_y(cfg.button_y_size * i)).color(color).mix(0);
            }
        }
    }
}
