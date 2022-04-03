#include "main.h"

#include "game/map.h"
#include "game/particles.h"
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

        [[nodiscard]] bool released() const
        {
            bool ok = false;
            for (const Input::Button &button : buttons)
            {
                if (button.down())
                    return false;
                if (button.released())
                    ok = true;
            }
            return ok;
        }
    };

    Button left = {{Input::left, Input::a}};
    Button right = {{Input::right, Input::d}};
    Button jump = {{Input::up, Input::w, Input::space, Input::c, Input::j}};
    Button timeshift = {{Input::z, Input::l}};
};
Controls con;

// Note, this structure is copied into timelines...
struct Player
{
    inline static const std::vector<ivec2> hitbox = {
        ivec2(-5, -9), ivec2(4, -9),
        ivec2(-5,  0), ivec2(4,  0),
        ivec2(-5,  8), ivec2(4,  8),
    };

    inline static const std::vector<ivec2> spike_hitbox = {
        ivec2(-5,  0), ivec2(4,  0),
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

    [[nodiscard]] bool VisibleAsGhost() const
    {
        return !dead && !in_prison;
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

    bool dead = false;
    int death_timer = 0;

    int anim_state = 0;
    int anim_variant = 0;

    bool in_prison = true;
    int prison_hp_left = 3;
    ivec2 prison_sprite_offset;
    int prison_anim_timer = 0;

    // This is here, because we need to save it to the timeline.
    float lava_y = 0;
};

struct Ghost
{
    int time_start = 0;
    std::vector<Player> states;

    bool prev_visible = false;
};
struct TimeManager
{
    std::vector<Ghost> ghosts;
    int time = 0;

    bool shifting_now = false;

    float shifting_speed = 0;
    float shifting_lag = 0;

    float positive_speed = 1; // Used to slowly gain time speed after shift.

    float shifting_effects_alpha = 0;

    void NextTimeline()
    {
        ghosts.emplace_back();
        ghosts.back().time_start = time;
    }

    void SavePlayer(const Player &p)
    {
        if (ghosts.empty())
            NextTimeline();
        ghosts.back().states.push_back(p);
    }

    void AddGhostParticles(ParticleController &par)
    {
        const Ghost *last_ghost = FindNewestGhost();

        for (Ghost &ghost : ghosts)
        {
            if (ghost.states.empty())
                continue;
            if (&ghost == last_ghost)
                continue;

            int rel_time = time - ghost.time_start;

            bool visible = time >= ghost.time_start && rel_time < int(ghost.states.size()) && ghost.states[rel_time].VisibleAsGhost();

            if (visible != ghost.prev_visible)
            {
                const Player &state = ghost.states[clamp(rel_time, 0, int(ghost.states.size()) - 1)];
                ghost.prev_visible = visible;
                for (int i = 0; i < 24; i++)
                {
                    fvec2 dir = fvec2::dir(ra.angle());
                    float dist = ra.f <= 1;
                    float c = ra.f <= 1;
                    fvec3 color(1 - c * 0.5f, 1 - c * 0.25f, 0);
                    if (ra.boolean())
                        std::swap(color.x, color.z);
                    par.Add(adjust(Particle{}, s.pos = state.pos + dir * dist * fvec2(4, 8), damp = 0.015, life = 20 <= ra.i <= 60, size = 1 <= ra.i <= 5, s.vel = dir * dist * 0.15 + state.prev_vel * 0.05, end_size = 1, color = color, beta = 0));
                }
            }
        }
    }

    void RenderGhosts(ivec2 camera_pos) const
    {
        static const auto &pl_region = texture_atlas.Get("player.png");
        constexpr ivec2 pl_size(36);

        const Ghost *last_ghost = FindNewestGhost();

        for (const Ghost &ghost : ghosts)
        {
            if (&ghost == last_ghost)
                continue; // Skip the last ghost.

            if (time < ghost.time_start)
                continue; // Too early.
            int rel_time = time - ghost.time_start;
            if (rel_time >= int(ghost.states.size()))
                continue; // Too late.

            if (!ghost.states[rel_time].VisibleAsGhost())
                continue; // Invisible, possibly dead.

            constexpr int max_time_offset = 3;

            for (int i = -max_time_offset; i <= max_time_offset; i++)
            {
                int this_rel_time = rel_time + i;
                if (this_rel_time < 0 || this_rel_time >= int(ghost.states.size()))
                    continue; // The time for this sprite is out of range.

                float alpha = 0.45 - abs(i) * 0.1;

                fvec3 color(1 - i / 4.f, 1 - i / 12.f, 0);
                if (i > 0)
                    std::swap(color.x, color.z);

                const Player &p = ghost.states[this_rel_time];

                r.iquad(p.pos - camera_pos, pl_region.region(pl_size * ivec2(p.anim_variant, p.anim_state), pl_size)).center().color(color).mix(0).alpha(alpha).beta(0).flip_x(p.facing_left);
            }
        }
    }

    // Find newest ghost for the current time.
    // Returns null on failure.
    const Ghost *FindNewestGhost() const
    {
        for (std::size_t i = ghosts.size(); i-- > 0;)
        {
            const Ghost &ghost = ghosts[i];
            if (ghost.time_start <= time)
            {
                int rel_time = time - ghost.time_start;
                if (rel_time >= int(ghost.states.size()))
                    return nullptr; // Note, not `continue`. This is more sane.
                return &ghost;
            }
        }
        return nullptr;
    }

    // Find newest player state for the current time.
    // Returns null on failure.
    const Player *FindNewestState() const
    {
        const Ghost *ret = FindNewestGhost();
        if (ret)
            return &ret->states[time - ret->time_start];
        return nullptr;
    }
};

namespace States
{
    STRUCT( World EXTENDS StateBase )
    {
        MEMBERS()

        Map map = Stream::ReadOnlyData(Program::ExeDir() + "map.json");

        Player p;
        ParticleController par = true;
        ParticleController par_timeless = false;
        TimeManager time;

        ivec2 camera_pos;

        bool buffered_jump = false;

        float fade = 1;
        bool have_timeshift_ability = false;

        std::string ability_message, ability_message2;
        int ability_timer = 0;

        bool seen_hint_death_rollback = false;
        float hint_death_hollback = 0;

        World()
        {
            p.lava_y = map.initial_lava_level;

            if (map.debug_player_start)
            {
                p.pos = *map.debug_player_start;
                p.in_prison = false;
            }
            else
            {
                p.pos = map.player_start;
            }

            if (map.debug_start_with_timeshift)
                have_timeshift_ability = true;

        }

        void Tick(std::string &next_state) override
        {
            (void)next_state;

            constexpr float gravity = 0.1, low_jump_gravity = 0.3;

            bool positive_time_step_this_tick = false;

            // Timeless particles.
            par_timeless.Tick(camera_pos);

            { // Gui. (should be nearly first)
                // Abilities.
                if (ability_timer > 0)
                {
                    ability_timer++;
                    if (ability_timer > 500)
                        ability_timer = 0;
                    return;
                }

                // Hints.
                if (!seen_hint_death_rollback && p.dead && have_timeshift_ability)
                {
                    if (time.shifting_now)
                        seen_hint_death_rollback = true;
                    clamp_var_max(hint_death_hollback += 0.01f);
                }
                else
                {
                    clamp_var_min(hint_death_hollback -= 0.01f);
                }
            }

            { // Time.
                bool timeshift_button_down = con.timeshift.down();
                bool should_timeshift = time.shifting_now;
                if (timeshift_button_down && !p.in_prison && have_timeshift_ability)
                    should_timeshift = true;
                else if (time.shifting_speed < 0.01)
                    should_timeshift = false;

                if (should_timeshift != time.shifting_now)
                {
                    time.shifting_now = should_timeshift;
                    time.shifting_lag = 0;
                    time.shifting_speed = 0;
                    time.positive_speed = 0;

                    if (!should_timeshift)
                    {
                        time.NextTimeline();
                        Sounds::time_start();
                    }
                    else
                    {
                        Sounds::time_stop();
                    }
                }

                clamp_var(time.shifting_effects_alpha += time.shifting_now ? 0.013 : -0.008);


                if (!time.shifting_now)
                {
                    if (time.positive_speed >= 0.99f)
                    {
                        time.positive_speed = 1;
                        positive_time_step_this_tick = true;
                    }
                    else
                    {
                        clamp_var_max(time.positive_speed += 0.01f);

                        time.shifting_lag += time.positive_speed;
                        int num_steps = time.shifting_lag;
                        time.shifting_lag -= num_steps;
                        if (num_steps > 0) // Yep.
                            positive_time_step_this_tick = true;
                    }

                    // Block time when the player is dead. But keep the death timer.
                    if (positive_time_step_this_tick && p.dead && p.death_timer > 60)
                    {
                        positive_time_step_this_tick = false;
                        p.death_timer++;
                    }

                    if (positive_time_step_this_tick)
                    {
                        time.time++;

                        time.AddGhostParticles(par_timeless);

                        // Particles.
                        par.Tick(camera_pos);
                    }
                }
                else
                {
                    if (timeshift_button_down)
                        clamp_var_max(time.shifting_speed += 0.02, time.time == 0 ? 0 : 4);
                    else
                        time.shifting_speed *= 0.97;

                    time.shifting_lag += time.shifting_speed;
                    int num_shifts = time.shifting_lag;
                    time.shifting_lag -= num_shifts;

                    while (num_shifts-- > 0 && time.time > 0)
                    {
                        time.time--;
                        time.AddGhostParticles(par_timeless);
                        par.ReverseTick();
                    }
                }
            }

            // Player.
            if (!time.shifting_now && con.jump.pressed())
                buffered_jump = true;
            if (positive_time_step_this_tick)
            {
                constexpr float
                    max_speed_x = 4,
                    max_speed_y_up = 3,
                    max_speed_y_down = 3.5,
                    walk_acc = 0.4,
                    walk_dec = 0.2,
                    walk_speed = 1.5,
                    jump_speed = 3.56,
                    vel_lag_damp = 0.01;

                { // Save to the timeline. (should be first?)
                    if (!p.in_prison)
                        time.SavePlayer(p);
                }

                bool controllable = !p.dead && !p.in_prison;

                // Status.
                p.prev_ground = p.ground;
                p.ground = p.SolidAtOffset(map, ivec2(0, 1));
                if (p.ground && !p.prev_ground && controllable)
                {
                    Sounds::landing(clamp((p.prev_vel.y - 1) / 3));

                    float v = clamp((p.prev_vel.y - 1) / 3) * 0.7;
                    if (v > 0.1)
                    {
                        for (int i = 0; i < 8; i++)
                        {
                            float x = ra.f.abs() <= 1;
                            fvec3 color(1, ra.f <= 1, 0);
                            fvec2 pos = p.pos with(y += 8 <= ra.f <= 10, x += x * 3);
                            par.Add(adjust(Particle{}, s.pos = pos, damp = 0.01, life = 20 <= ra.i <= 40, size = 1 <= ra.i <= 3, s.vel = v * fvec2(p.prev_vel.x * 0.8 + pow(abs(x), 2) * sign(x) * 2, -0.9 <= ra.f <= 0), end_size = 1, color = color));
                        }
                    }
                }

                { // Prison controls.
                    if (p.in_prison)
                    {
                        if (con.jump.pressed())
                        {
                            p.prison_hp_left--;
                            if (p.prison_hp_left <= 0)
                            {
                                p.in_prison = false;
                                Sounds::broke_prison();
                            }
                            else
                            {
                                Sounds::breaking_prison();
                            }

                            p.prison_sprite_offset = p.prison_hp_left == 1 ? ivec2(1, 0) : ivec2(-1, 0);
                            p.prison_anim_timer = 10;
                        }
                    }
                    if (p.prison_anim_timer > 0)
                        p.prison_anim_timer--;
                }

                // Controls.
                if (controllable)
                {
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
                    if (p.ground && buffered_jump)
                    {
                        p.vel.y = -jump_speed;
                        Sounds::jump();

                        for (int i = 0; i < 8; i++)
                        {
                            float x = ra.f.abs() <= 1;
                            fvec3 color(1, ra.f <= 1, 0);
                            fvec2 pos = p.pos with(y += 8 <= ra.f <= 10, x += x * 3);
                            par.Add(adjust(Particle{}, s.pos = pos, damp = 0.01, life = 20 <= ra.i <= 40, size = 1 <= ra.i <= 3, s.vel = fvec2(p.prev_vel.x * 0.4 + pow(abs(x), 2) * sign(x) * 0.5, -1.2 <= ra.f <= 0), end_size = 1, color = color));
                        }
                    }

                    // Walking particles.
                    if (p.walking_timer % 10 == 5)
                    {
                        fvec3 color(1, ra.f <= 1, 0);
                        fvec2 pos = p.pos with(y += 8 <= ra.f <= 10, x += ra.f.abs() <= 1);
                        par.Add(adjust(Particle{}, s.pos = pos, damp = 0.01, life = 20 <= ra.i <= 40, size = 1 <= ra.i <= 3, s.vel = fvec2(p.prev_vel.x * 0.3 + (ra.f.abs() <= 0.2), -0.7 <= ra.f <= 0), end_size = 1, color = color));
                    }
                }

                // Gravity.
                if (controllable)
                {
                    if (p.vel.y < 0 && !con.jump.down())
                        p.vel.y += low_jump_gravity;
                    else
                        p.vel.y += gravity;
                }

                // Apply velocity.
                if (controllable)
                {
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

                // Getting abilities.
                if (controllable)
                {
                    auto PickUpAbility = [&](std::optional<ivec2> &ability, std::string_view text, std::string_view text2) -> bool
                    {
                        if (!ability)
                            return false;
                        if ((abs(*ability - p.pos) < ivec2(6,10)).all())
                        {
                            for (int i = 0; i < 24; i++)
                            {
                                fvec2 dir = fvec2::dir(ra.angle());
                                float dist = ra.f <= 1;
                                float c = ra.f <= 1;
                                fvec3 color(1 - c * 0.5f, 1 - c * 0.25f, 0);
                                if (ra.boolean())
                                    std::swap(color.x, color.z);
                                par_timeless.Add(adjust(Particle{}, s.pos = *ability + dir * dist * 6, damp = 0.005, life = 20 <= ra.i <= 60, size = 1 <= ra.i <= 7, s.vel = dir * dist * 0.35, end_size = 1, color = color, beta = 0));
                            }
                            ability.reset();
                            Sounds::got_item();
                            ability_timer = 1;
                            ability_message = text;
                            ability_message2 = text2;
                            return true;
                        }
                        return false;
                    };

                    if (PickUpAbility(map.ability_timeshift, "Timeshift", "Hold Z / L to travel back in time"))
                        have_timeshift_ability = true;
                }

                // Death conditions.
                if (controllable)
                {
                    // Spikes.
                    for (ivec2 offset : p.spike_hitbox)
                        if (map.at_pixel(p.pos + offset).info().kills)
                            p.dead = true;

                    // Lava.
                    if (p.pos.y > p.lava_y + 4)
                        p.dead = true;
                }

                // Death effects.
                if (p.dead)
                {
                    if (p.death_timer == 1)
                        Sounds::death();

                    float t = clamp_min(1 - p.death_timer / 15.f);

                    if (t > 0)
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            fvec2 dir = fvec2::dir(ra.angle());
                            float dist = ra.f <= 1;
                            fvec3 color(1, ra.f <= 1, 0);
                            par.Add(adjust(Particle{}, s.pos = p.pos + dir * dist * fvec2(8, 12), damp = 0, life = 60 <= ra.i <= 180, size = 1 <= ra.i <= 1+t*7, s.vel = dir * dist * pow(1 - t, 1.5) * 4 + p.prev_vel * 0.05, end_size = 1, color = color));
                        }
                    }
                }

                { // Timers.
                    if (p.dead)
                        p.death_timer++;

                    if (!p.in_prison && time.time % 10 == 0)
                        p.lava_y -= 1;

                    buffered_jump = false;
                }

                { // Determining the current sprite.
                    constexpr int
                    breathing_variant_len = 20,
                    running_variant_len = 8, running_num_variants = 4;

                    constexpr float jumping_variant_speeds[] = {-1, -0.5, 0.5};

                    if (!p.ground)
                    {
                        // Jumping.
                        p.anim_state = 2;
                        p.anim_variant = std::lower_bound(std::begin(jumping_variant_speeds), std::end(jumping_variant_speeds), p.vel.y) - std::begin(jumping_variant_speeds);
                    }
                    else if (p.is_walking)
                    {
                        // Running.
                        p.anim_state = 1;
                        p.anim_variant = p.walking_timer / running_variant_len % running_num_variants;
                    }
                    else
                    {
                        // Breathing.
                        p.anim_state = 0;
                        constexpr int breathing_anim[] = {0, 0, 0, 1, 2, 2, 2, 1};
                        p.anim_variant = breathing_anim[time.time / breathing_variant_len % std::size(breathing_anim)];
                    }
                }
            }
            else if (time.shifting_now)
            {
                // Try restoring the state from timeline.
                if (const Player *state = time.FindNewestState())
                    p = *state;

                buffered_jump = false;
            }

            { // Fade.
                bool death_fade = p.dead && p.death_timer >= 60 && !have_timeshift_ability;
                if (death_fade)
                {
                    clamp_var_max(fade += 0.01f);
                    if (fade >= 1)
                        next_state = "World{}";
                }
                else
                {
                    clamp_var_min(fade -= 0.01f);
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

            { // Prison.
                static const auto &region = texture_atlas.Get("prison.png");
                static const ivec2 size = region.size with(y /= 2);

                ivec2 prison_pos = map.player_start - camera_pos;
                if ((abs(prison_pos) <= (screen_size + size) / 2).all())
                {
                    r.iquad(prison_pos + p.prison_sprite_offset * (p.prison_anim_timer > 0), region.region(ivec2(0, size.y * !p.in_prison), size)).center();
                }
            }

            { // Abilities.
                static const auto &region = texture_atlas.Get("cube.png");

                constexpr int offset_array[] = {0, -1, -1, -1, 0, 1, 1, 1};
                constexpr int offset_len = 20;
                int offset = offset_array[(time.time / offset_len) % std::size(offset_array)];

                auto DrawAbility = [&](const std::optional<ivec2> &pos)
                {
                    if (!pos)
                        return;
                    ivec2 screen_pos = *pos - camera_pos;
                    if ((abs(screen_pos) > screen_size / 2 + 16).any())
                        return;

                    r.iquad(screen_pos with(y += offset), region).center();
                };
                DrawAbility(map.ability_timeshift);
            }

            { // Player ghosts.
                time.RenderGhosts(camera_pos);
            }

            { // Player.
                static const auto &pl_region = texture_atlas.Get("player.png");
                constexpr ivec2 pl_size(36);

                float alpha = p.in_prison ? 0 : clamp_min(1 - p.death_timer / 10.f);

                if (alpha > 0.001f)
                    r.iquad(p.pos - camera_pos, pl_region.region(pl_size * ivec2(p.anim_variant, p.anim_state), pl_size)).center().flip_x(p.facing_left).alpha(alpha);
            }

            { // Lava.
                static const auto &lava_region = texture_atlas.Get("lava.png");

                int anim_x = time.time / 4 % lava_region.size.x;

                // Repeating top part.
                int x = div_ex(camera_pos.x - screen_size.x / 2 + anim_x, lava_region.size.x) * lava_region.size.x;
                for (; x < camera_pos.x + anim_x + screen_size.x / 2; x += lava_region.size.x)
                    r.iquad(ivec2(x, p.lava_y) - camera_pos with(x += anim_x), lava_region);

                // Bottom part as one large rect.
                int bottom_y = p.lava_y + lava_region.size.y - camera_pos.y;
                if (bottom_y < screen_size.y / 2)
                    r.iquad(ivec2(-screen_size.x/2, bottom_y), screen_size/2).absolute().tex(lava_region.pos + fvec2(0.5, lava_region.size.y - 0.5), fvec2());
            }

            // Map.
            map.render(camera_pos);

            // Timeless particles.
            par_timeless.Render(camera_pos);

            // Particles.
            par.Render(camera_pos);

            { // Time machine.
                constexpr int num_rays = 64;
                static const float
                    dist_min = 192,
                    dist_max = (screen_size / 2).len() + 16;

                float t = smoothstep(time.shifting_effects_alpha);

                if (t > 0)
                {
                    float alpha1 = t * 0.3f;
                    float alpha2 = t * 0.7f;

                    constexpr int maxdelta = 2;

                    float rot_speed = (time.shifting_now ? time.shifting_speed : time.positive_speed);

                    float height = 1;

                    for (int delta = -maxdelta; delta <= maxdelta; delta++)
                    {
                        fvec3 color(1 - delta / 4.f, 1 - delta / 12.f, 0);
                        if (delta > 0)
                            std::swap(color.x, color.z);

                        for (int i = 0; i < num_rays; i++)
                        {
                            float angle = time.time * 0.001f - delta * rot_speed * 0.003f + i * 2 * f_pi / num_rays;

                            r.fquad(fvec2(), fvec2(dist_max - dist_min, height)).center(fvec2(-dist_min, height/2)).rotate(angle).color(color).alpha(alpha1, alpha2, alpha2, alpha1).beta(0);
                        }
                    }
                }
            }

            { // Gui.
                // Ability messages.
                if (ability_timer > 0)
                {
                    constexpr int ticks_per_letter = 8;

                    int bg_time = 30;
                    int first_line_time = 60;

                    ivec2 text_size = Graphics::Text(Fonts::main, ability_message).ComputeStats().size;
                    Graphics::Text text(Fonts::main, ability_message.substr(0, clamp_min((ability_timer - first_line_time) / ticks_per_letter)));
                    Graphics::Text text2(Fonts::main, ability_message2);

                    r.iquad(ivec2(0, 0), ivec2(max(text_size.x, text2.ComputeStats().size.x) + 12, 64)).color(fvec3(0)).center().alpha(smoothstep(clamp((ability_timer - bg_time) / 30.f)));
                    r.itext(-text_size/2 + ivec2(0, -8), text).align_x(-1).color(fvec3(255, 179, 26) / 255);

                    int second_line_time = first_line_time + int(ability_message.size()) * ticks_per_letter + 30;

                    if (ability_timer > second_line_time)
                        r.itext(ivec2(0, 12), text2).alpha(clamp_max((ability_timer - second_line_time) / 60.f)).color(fvec3(255, 76, 5) / 255);
                }

                { // Hints.
                    auto ShowHint = [&](std::string_view message, float t)
                    {
                        if (t < 0.001f)
                            return;

                        Graphics::Text text(Fonts::main, message);
                        float alpha = smoothstep(clamp(t));

                        for (int i = 0; i < 4; i++)
                            r.itext(ivec2(0, screen_size.y/2) + ivec2::dir4(i), text).align_y(1).alpha(alpha * 0.5f).color(fvec3(0));
                        r.itext(ivec2(0, screen_size.y/2), text).align_y(1).alpha(alpha).color(fvec3(255, 179, 26) / 255);
                    };

                    ShowHint("Hold Z / L to travel back in time", (p.death_timer - 90) / 60.f);
                }
            }

            { // Fade.
                if (fade > 0.001f)
                    r.iquad(ivec2(), screen_size).center().color(fvec3(0)).alpha(smoothstep(fade));
            }

            { // Vignette.
                static const auto &region = texture_atlas.Get("vignette.png");
                r.iquad(ivec2(), region).alpha(0.35).center();
            }

            r.Finish();
        }
    };
}
