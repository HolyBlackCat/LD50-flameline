#include "main.h"

#include "game/map.h"

namespace States
{
    STRUCT( World EXTENDS StateBase )
    {
        MEMBERS()

        Map map = Stream::ReadOnlyData(Program::ExeDir() + "map.json");

        int lava_y = (map.size().y - 8) * tile_size;

        ivec2 camera_pos{};

        void Tick(std::string &next_state) override
        {
            (void)next_state;
            camera_pos = mouse.pos() * 2;
        }

        void Render() const override
        {
            Graphics::SetClearColor(fvec3(0));
            Graphics::Clear();

            r.BindShader();

            { // Background.
                static const auto &bg_region = texture_atlas.Get("bg.png");

                constexpr float bg_speed_factor = 0.6f;
                ivec2 bg_camera_pos = iround(camera_pos * bg_speed_factor);

                ivec2 corner_a = div_ex(bg_camera_pos - screen_size / 2, bg_region.size);
                ivec2 corner_b = div_ex(bg_camera_pos + screen_size / 2, bg_region.size);
                for (ivec2 tile_pos : corner_a <= vector_range <= corner_b)
                {
                    r.iquad(tile_pos * bg_region.size - bg_camera_pos, bg_region);
                }
            }

            { // Lava.
                static const auto &lava_region = texture_atlas.Get("lava.png");

                int anim_x = window.Ticks() / 4 % lava_region.size.x;

                // Repeating top part.
                int x = div_ex(camera_pos.x - screen_size.x / 2 + anim_x, lava_region.size.x) * lava_region.size.x;
                for (; x < camera_pos.x + anim_x + screen_size.x / 2; x += lava_region.size.x)
                    r.iquad(ivec2(x, lava_y) - camera_pos with(x += anim_x), lava_region);

                // Bottom part as one large rect.
                int bottom_y = lava_y + lava_region.size.y - camera_pos.y;
                if (bottom_y < screen_size.y / 2)
                    r.iquad(ivec2(-screen_size.x/2, bottom_y), screen_size/2).absolute().tex(lava_region.pos + fvec2(0.5, lava_region.size.y - 0.5), fvec2());
            }

            // Map.
            map.render(camera_pos);

            r.Finish();
        }
    };
}
