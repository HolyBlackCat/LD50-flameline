#include "main.h"

#include "game/map.h"
#include "game/sounds.h"

struct Controls
{
    struct Button
    {
        std::vector<Input::Button> buttons;

        [[nodiscard]] bool down() const
        {
            for (const Input::Button &button : buttons)
                if (button.down())
                    return true;
            return false;
        }

        [[nodiscard]] bool pressed() const
        {
            bool ok = false;
            for (const Input::Button &button : buttons)
            {
                if (button.down() && !button.pressed())
                    return false;
                if (button.pressed())
                    ok = true;
            }
            return ok;
        }
    };

    Button left = {{Input::left, Input::a}};
    Button right = {{Input::right, Input::d}};
    Button jump = {{Input::space, Input::x, Input::j, Input::w}};
};
Controls con;

struct Player
{
    inline static const std::vector<ivec2> hitbox = {
        ivec2(-5,  -7), ivec2(4,  -7),
        ivec2(-5,   0), ivec2(4,   0),
        ivec2(-5,  11), ivec2(4,  11),
    };

    [[nodiscard]] static bool SolidAtPos(const Map &map, ivec2 pos)
    {
        for (ivec2 point : hitbox)
            if (map.at_pixel(pos + point).info().solid)
                return true;
        return false;
    }

    [[nodiscard]] bool SolidAtOffset(const Map &map, ivec2 offset) const
    {
        return SolidAtPos(map, pos + offset);
    }

    ivec2 pos;
    fvec2 vel;
    fvec2 prev_vel;
    fvec2 vel_lag;

    bool ground = false;
    bool prev_ground = false;

    bool facing_left = false;

    bool is_walking = false;
    int walking_timer = 0;
};

namespace States
{
    STRUCT( World EXTENDS StateBase )
    {
        MEMBERS()

        Map map = Stream::ReadOnlyData(Program::ExeDir() + "map.json");

        Player p;

        int lava_y = (map.size().y - 8) * tile_size;

        ivec2 camera_pos;

        World()
        {
            p.pos = map.player_start;
        }

        void Tick(std::string &next_state) override
        {
            (void)next_state;

            constexpr float gravity = 0.1, low_jump_gravity = 0.3;

            { // Player.
                constexpr float
                    max_speed_x = 4,
                    max_speed_y_up = 3,
                    max_speed_y_down = 3,
                    walk_acc = 0.5,
                    walk_dec = 0.2,
                    walk_speed = 2,
                    jump_speed = 3.56,
                    vel_lag_damp = 0.01;

                // Status.
                p.prev_ground = p.ground;
                p.ground = p.SolidAtOffset(map, ivec2(0, 1));
                if (p.ground && !p.prev_ground)
                    Sounds::landing(clamp((p.prev_vel.y - 1) / 3));


                { // Controls.
                    // Walking.
                    int hc = con.right.down() - con.left.down();
                    if (hc && p.vel.x * hc < walk_speed)
                        p.vel.x = clamp_max(p.vel.x * hc + walk_acc, walk_speed) * hc;
                    else
                        p.vel.x = clamp_min(abs(p.vel.x) - walk_dec, 0) * sign(p.vel.x);

                    // Walking anim states.
                    if (hc)
                        p.facing_left = hc < 0;
                    if (hc && p.ground)
                        p.is_walking = !p.SolidAtOffset(map, ivec2(hc, 0));
                    else
                        p.is_walking = false;
                    if (p.is_walking)
                        p.walking_timer++;
                    else
                        p.walking_timer = 0;


                    // Jumping.
                    if (p.ground && con.jump.pressed())
                    {
                        p.vel.y = -jump_speed;
                        Sounds::jump();
                    }
                }

                // Gravity.
                if (p.vel.y < 0 && !con.jump.down())
                    p.vel.y += low_jump_gravity;
                else
                    p.vel.y += gravity;

                { // Apply velocity.
                    p.prev_vel = p.vel;

                    fvec2 clamped_vel = clamp(p.vel, ivec2(-max_speed_x, -max_speed_y_up), ivec2(max_speed_x, max_speed_y_down));

                    fvec2 eff_vel = clamped_vel + p.vel_lag;
                    ivec2 int_vel = iround(eff_vel);
                    p.vel_lag = eff_vel - int_vel;

                    p.vel_lag *= 1 - vel_lag_damp;

                    while (int_vel)
                    {
                        for (int i = 0; i < 2; i++)
                        {
                            if (int_vel[i] == 0)
                                continue;

                            int s = sign(int_vel[i]);
                            int_vel[i] -= s;

                            ivec2 offset;
                            offset[i] = s;

                            if (!p.SolidAtOffset(map, offset))
                            {
                                p.pos[i] += s;
                            }
                            else
                            {
                                int_vel[i] = 0;
                                if (p.vel[i] * s > 0)
                                    p.vel[i] = 0;
                                if (p.vel_lag[i] * s > 0)
                                    p.vel_lag[i] = 0;
                            }
                        }
                    }
                }
            }

            { // Camera.
                camera_pos = p.pos;

                float audio_dist = 3;

                Audio::Source::DefaultRefDistance(screen_size.x * audio_dist);
                Audio::ListenerPosition(fvec3(0, 0, -screen_size.x * audio_dist));
                Audio::ListenerOrientation(fvec3(0, 0, 1), fvec3(0, -1, 0));
            }
        }

        void Render() const override
        {
            Graphics::SetClearColor(fvec3(0));
            Graphics::Clear();

            r.BindShader();

            { // Background.
                static const auto &bg_region = texture_atlas.Get("bg.png");

                constexpr float bg_speed_factor = 0.5f;
                ivec2 bg_camera_pos = iround(camera_pos * bg_speed_factor);

                ivec2 corner_a = div_ex(bg_camera_pos - screen_size / 2, bg_region.size);
                ivec2 corner_b = div_ex(bg_camera_pos + screen_size / 2, bg_region.size);
                for (ivec2 tile_pos : corner_a <= vector_range <= corner_b)
                {
                    r.iquad(tile_pos * bg_region.size - bg_camera_pos, bg_region);
                }
            }

            { // Player
                static const auto &pl_region = texture_atlas.Get("player.png");
                constexpr ivec2 pl_size(36);

                constexpr int
                    breathing_variant_len = 20,
                    running_variant_len = 8, running_num_variants = 4;

                constexpr float jumping_variant_speeds[] = {-1, -0.5, 0.5};

                int anim_state = 0;
                int anim_variant = 0;

                if (!p.ground)
                {
                    // Jumping.
                    anim_state = 2;
                    anim_variant = std::lower_bound(std::begin(jumping_variant_speeds), std::end(jumping_variant_speeds), p.vel.y) - std::begin(jumping_variant_speeds);
                }
                else if (p.is_walking)
                {
                    // Running.
                    anim_state = 1;
                    anim_variant = p.walking_timer / running_variant_len % running_num_variants;
                }
                else
                {
                    // Breathing.
                    anim_state = 0;
                    constexpr int breathing_anim[] = {0, 0, 0, 1, 2, 2, 2, 1};
                    anim_variant = breathing_anim[window.Ticks() / breathing_variant_len % std::size(breathing_anim)];
                }

                r.iquad(p.pos - camera_pos, pl_region.region(pl_size * ivec2(anim_variant, anim_state), pl_size)).center().flip_x(p.facing_left);
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
