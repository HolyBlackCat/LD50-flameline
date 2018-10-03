#include "program/parachute.h"

#include <utility>
#include <vector>

#include "game/states/menu/buttons.h"
#include "game/meta.h"
#include "game/controls.h"
#include "game/states/menu/config.h"

namespace States::Details::Menu
{
    static const TextureAtlas::Image button_images[Buttons::button_count]
    {
        Atlas().Get("gui/menu.button.new_game.png"),
        Atlas().Get("gui/menu.button.quit.png"),
    };



    Buttons::Buttons() {}

    void Buttons::Tick()
    {
        // Select new button
        int change = controls.down.pressed() - controls.up.pressed();
        active_button = (active_button + button_count + change) % button_count;

        // Change button visuals
        for (int i = 0; i < button_count; i++)
        {
            auto &button = buttons[i];
            clamp_var(button.time += ((active_button == i)*2-1) * Cfg::menu->button_time_step);
        }
    }

    void Buttons::Render() const
    {
        for (int i = 0; i < button_count; i++)
        {
            const auto &button = buttons[i];
            const auto &img = button_images[i];

            for (int j = 0; j < 2; j++)
            {
                fvec3 color;
                if (j)
                    color = button.time * Cfg::menu->button_color_active + (1-button.time) * Cfg::menu->button_color_main;
                else
                    color = Cfg::menu->button_color_shadow;

                render.iquad(Cfg::menu->button_offset + ivec2(-img.size.x/2, i * Cfg::menu->button_y_step) + (1-j), img.size).tex(img.pos).color(color).mix(0);
            }
        }
    }
}
