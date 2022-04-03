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
        return !dead;
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

    // This is here, because we need to save it to the timeline.
    float lava_y = 0;
};

struct Ghost
{
    int time_start = 0;
    std::vector<Player> states;
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

    void AddGhostParticles(ParticleController &par) const
    {
        const Ghost *last_ghost = FindNewestGhost();

        for (const Ghost &ghost : ghosts)
        {
            if (&ghost == last_ghost)
                continue;
            if (ghost.states.empty())
                continue;

            if (ghost.time_start == time || ghost.time_start + int(ghost.states.size()) - 1 == time)
            {
                const Player &state = ghost.states[time - ghost.time_start];

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

        World()
        {
            p.pos = map.player_start;
            p.lava_y = map.initial_lava_level;
        }

        void Tick(std::string &next_state) override
        {
            (void)next_state;

            constexpr float gravity = 0.1, low_jump_gravity = 0.3;

            bool positive_time_step_this_tick = false;

            // Timeless particles.
            par_timeless.Tick(camera_pos);

            { // Time.
                bool timeshift_button_down = con.timeshift.down();
                bool should_timeshift = time.shifting_now;
                if (timeshift_button_down)
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
                        time.NextTimeline();
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
                        clamp_var_max(time.shifting_speed += 0.02, 4);
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
                    time.SavePlayer(p);
                }

                // Status.
                p.prev_ground = p.ground;
                p.ground = p.SolidAtOffset(map, ivec2(0, 1));
                if (p.ground && !p.prev_ground)
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

                // Controls.
                if (!p.dead)
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
                if (!p.dead)
                {
                    if (p.vel.y < 0 && !con.jump.down())
                        p.vel.y += low_jump_gravity;
                    else
                        p.vel.y += gravity;
                }

                // Apply velocity.
                if (!p.dead)
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

                // Death conditions.
                if (!p.dead)
                {
                    // Spikes.
                    for (ivec2 offset : p.spike_hitbox)
                        if (map.at_pixel(p.pos + offset).info().kills)
                            p.dead = true;

                    // Lava.
                    if (p.pos.y > p.lava_y + 12)
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

                    if (time.time % 10 == 0)
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

            { // Player ghosts.
                time.RenderGhosts(camera_pos);
            }

            { // Player.
                static const auto &pl_region = texture_atlas.Get("player.png");
                constexpr ivec2 pl_size(36);

                float alpha = clamp_min(1 - p.death_timer / 10.f);

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

                    for (int delta = -maxdelta; delta <= maxdelta; delta++)
                    {
                        fvec3 color(1 - delta / 4.f, 1 - delta / 12.f, 0);
                        if (delta > 0)
                            std::swap(color.x, color.z);

                        for (int i = 0; i < num_rays; i++)
                        {
                            float angle = time.time * 0.001f + delta * (time.shifting_now ? time.shifting_speed : time.positive_speed) * 0.003f + i * 2 * f_pi / num_rays;

                            r.fquad(fvec2(), fvec2(dist_max - dist_min, 1)).center(fvec2(-dist_min, 0.5f)).rotate(angle).color(color).alpha(alpha1, alpha2, alpha2, alpha1).beta(0);
                        }
                    }
                }
            }

            { // Vignette.
                static const auto &region = texture_atlas.Get("vignette.png");
                r.iquad(ivec2(), region).alpha(0.35).center();
            }

            r.Finish();
        }
    };
}
