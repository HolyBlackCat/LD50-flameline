#include "main.h"

#include "game/map.h"
#include "game/particles.h"
#include "game/sounds.h"

constexpr int max_timeshifts = 255;

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
    Button jump = {{Input::space, Input::c, Input::j}};
    Button shoot = {{Input::x, Input::k}};
    Button timeshift = {{Input::z, Input::l}};
};
Controls con;

struct Shot
{
    inline static const std::vector<ivec2> hitbox = {
        ivec2(-3, -3), ivec2(-3,  2),
        ivec2( 2, -3), ivec2( 2,  2),
    };

    static int GetAnimVariant(int time)
    {
        return time / 5 % 6;
    }

    fvec2 pos;
    fvec2 vel;
};

// Note, this structure is copied into timelines...
struct Player
{
    inline static const std::vector<ivec2> hitbox = {
        ivec2(-4, -9), ivec2(3, -9),
        ivec2(-4,  0), ivec2(3,  0),
        ivec2(-4,  8), ivec2(3,  8),
    };

    inline static const std::vector<ivec2> spike_hitbox = {
        ivec2(-4,  0), ivec2(3,  0),
    };

    static constexpr ivec2 shot_hitbox_halfsize = ivec2(8,8);

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

    std::optional<Shot> shot;

    int remaining_boost_frames = 0;
    fvec2 boost_vel;

    // This is here, because we need to save it to the timeline.
    float lava_y = 0;
};

struct Ghost
{
    int time_start = 0;
    std::vector<Player> states;

    bool prev_visible = false;
    bool prev_shot_visible = false;
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

    std::map<ivec2, int> block_breaking_times;

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

            bool shot_visible = visible && ghost.states[rel_time].shot;
            if (shot_visible != ghost.prev_shot_visible)
            {
                ghost.prev_shot_visible = shot_visible;

                // Try to guess the shot pos.
                int index = clamp(rel_time, 0, int(ghost.states.size()) - 1);
                std::optional<fvec2> shot_pos;
                if (const auto &opt = ghost.states[index].shot)
                    shot_pos = opt->pos;
                else if (index > 0 && ghost.states[index-1].shot)
                    shot_pos = ghost.states[index-1].shot->pos;
                else if (index + 1 < int(ghost.states.size()) && ghost.states[index+1].shot)
                    shot_pos = ghost.states[index+1].shot->pos;

                if (shot_pos)
                {
                    for (int i = 0; i < 12; i++)
                    {
                        fvec2 dir = fvec2::dir(ra.angle());
                        float dist = ra.f <= 1;
                        float c = ra.f <= 1;
                        fvec3 color(1 - c * 0.5f, 1 - c * 0.25f, 0);
                        if (ra.boolean())
                            std::swap(color.x, color.z);
                        par.Add(adjust(Particle{}, s.pos = *shot_pos + dir * dist * 2, damp = 0.015, life = 20 <= ra.i <= 60, size = 1 <= ra.i <= 3, s.vel = dir * dist * 0.15, end_size = 1, color = color, beta = 0));
                    }
                }
            }
        }
    }

    void RenderGhosts(ivec2 camera_pos) const
    {
        static const auto &pl_region = texture_atlas.Get("player.png");
        static const auto &shot_region = texture_atlas.Get("shot.png");
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

                { // Player.
                    ivec2 rel_pos = p.pos - camera_pos;
                    if ((rel_pos < screen_size / 2 + 24).all())
                        r.iquad(p.pos - camera_pos, pl_region.region(pl_size * ivec2(p.anim_variant, p.anim_state), pl_size)).center().color(color).mix(0).alpha(alpha).beta(0).flip_x(p.facing_left);
                }

                // Shot.
                if (p.shot)
                {
                    fvec2 rel_pos = p.shot->pos - camera_pos;
                    if ((rel_pos <= screen_size / 2 + 16).all())
                        r.fquad(rel_pos, shot_region.region(ivec2(shot_region.size.y * Shot::GetAnimVariant(time), 0), ivec2(shot_region.size.y))).center().flip_x(p.shot->vel.x < 0).color(color).mix(0).alpha(alpha).beta(0);
                }
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

    int RemainingShifts() const
    {
        return clamp(max_timeshifts - int(ghosts.size()) + !shifting_now, 0, max_timeshifts);
    }
};

namespace States
{
    STRUCT( World EXTENDS StateBase )
    {
        MEMBERS()

        Map map = Stream::ReadOnlyData(Program::ExeDir() + "map.json");
        Map map_orig = map;

        Player p;
        ParticleController par = true;
        ParticleController par_timeless = false;
        TimeManager time;

        ivec2 camera_pos;

        bool buffered_jump = false;

        float fade = 1;
        bool have_timeshift_ability = false;
        bool have_doublejump_ability = false;
        bool have_gun_ability = false;

        int time_since_got_timeshift = 0;

        std::string ability_message, ability_message2;
        int ability_timer = 0;

        bool seen_hint_death_rollback = false;
        float hint_death_hollback = 0;

        bool seen_hint_jump = false;
        float hint_jump = 0;

        struct Hint
        {
            ivec2 pos;
            std::string message;
            float alpha = 0;
        };
        std::vector<Hint> hints;

        World()
        {
            p.lava_y = map.initial_lava_level;

            map.points.ForEachPointWithNamePrefix("hint:", [&](std::string_view suffix, fvec2 pos)
            {
                Hint new_hint;
                new_hint.message = suffix;
                new_hint.pos = pos;
                hints.push_back(std::move(new_hint));
            });

            { // Debug features.
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
                if (map.debug_start_with_doublejump)
                    have_doublejump_ability = true;
                if (map.debug_start_with_gun)
                    have_gun_ability = true;
            }
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
                    if (ability_timer > 440)
                        ability_timer = 0;
                    return;
                }

                { // Hints.
                    auto ProcessHint = [](float &timer, bool increase)
                    {
                        clamp_var(timer += 0.01f * (increase ? 1 : -1));
                        return increase;
                    };

                    for (Hint &hint : hints)
                    {
                        ProcessHint(hint.alpha, !p.dead && ((p.pos - hint.pos).abs() < 12).all());
                    }

                    if (ProcessHint(hint_death_hollback, !seen_hint_death_rollback && p.dead && have_timeshift_ability && time.RemainingShifts() > 0 && p.death_timer > 90) && time.shifting_now)
                        seen_hint_death_rollback = true;
                    if (ProcessHint(hint_jump, !seen_hint_jump) && !p.in_prison)
                        seen_hint_jump = true;
                }
            }

            { // Time.
                bool timeshift_button_down = con.timeshift.down();
                bool should_timeshift = time.shifting_now;
                if (timeshift_button_down && !p.in_prison && have_timeshift_ability && (time.RemainingShifts() > 0 || time.shifting_now))
                    should_timeshift = true;
                else if (time.shifting_speed < 0.05)
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

            { // Restore block state from the timeline.
                for (const auto &[pos, break_time] : time.block_breaking_times)
                {
                    if (time.time < break_time)
                        map.at(pos).tile = map_orig.at(pos).tile;
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

                constexpr ivec2
                    ghost_hitbox_halfsize(12, 12);

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
                    if (buffered_jump)
                    {
                        bool can_jump = p.ground;
                        bool using_doublejump = false;

                        // Try to doublejump off of a ghost.
                        if (!can_jump && have_doublejump_ability)
                        {
                            const Ghost *newest_ghost = time.FindNewestGhost();
                            auto it = std::find_if(time.ghosts.begin(), time.ghosts.end(), [&](const Ghost &ghost)
                            {
                                if (&ghost == newest_ghost)
                                    return false;
                                if (time.time < ghost.time_start)
                                    return false;
                                int rel_time = time.time - ghost.time_start;
                                if (rel_time >= int(ghost.states.size()))
                                    return false;
                                const Player &state = ghost.states[rel_time];
                                if (!state.VisibleAsGhost())
                                    return false;
                                return (abs(state.pos - p.pos) < ghost_hitbox_halfsize).all();
                            });

                            if (it != time.ghosts.end())
                            {
                                for (std::size_t i = time.time - it->time_start; i < it->states.size(); i++)
                                    it->states[i].dead = true;

                                can_jump = true;
                                using_doublejump = true;
                            }
                        }

                        // Execute the jump.
                        if (can_jump)
                        {
                            p.vel.y = -jump_speed;
                            Sounds::jump();

                            if (!using_doublejump)
                            {
                                for (int i = 0; i < 8; i++)
                                {
                                    float x = ra.f.abs() <= 1;
                                    fvec3 color(1, ra.f <= 1, 0);
                                    fvec2 pos = p.pos with(y += 8 <= ra.f <= 10, x += x * 3);
                                    par.Add(adjust(Particle{}, s.pos = pos, damp = 0.01, life = 20 <= ra.i <= 40, size = 1 <= ra.i <= 3, s.vel = fvec2(p.prev_vel.x * 0.4 + pow(abs(x), 2) * sign(x) * 0.5, -1.2 <= ra.f <= 0), end_size = 1, color = color));
                                }
                            }
                            else
                            {
                                for (int i = 0; i < 24; i++)
                                {
                                    constexpr float half_max_angle = f_pi / 3.2f;

                                    fvec2 dir = fvec2::dir(f_pi/2 - half_max_angle <= ra.f <= f_pi/2 + half_max_angle);
                                    float dist = ra.f <= 1;
                                    float c = ra.f <= 1;
                                    fvec3 color(1 - c * 0.5f, 1 - c * 0.25f, 0);
                                    if (ra.boolean())
                                        std::swap(color.x, color.z);
                                    par.Add(adjust(Particle{}, s.pos = p.pos + (fvec2(-3, 6) <= ra.fvec2 <= fvec2(3, 10)), damp = 0.015, life = 20 <= ra.i <= 60, size = 1 <= ra.i <= 5, s.vel = dir * pow(dist, 1.9f) * 2, end_size = 1, color = color, beta = 0));
                                }
                            }
                        }
                    }

                    // Walking particles.
                    if (p.walking_timer % 10 == 5)
                    {
                        fvec3 color(1, ra.f <= 1, 0);
                        fvec2 pos = p.pos with(y += 8 <= ra.f <= 10, x += ra.f.abs() <= 1);
                        par.Add(adjust(Particle{}, s.pos = pos, damp = 0.01, life = 20 <= ra.i <= 40, size = 1 <= ra.i <= 3, s.vel = fvec2(p.prev_vel.x * 0.3 + (ra.f.abs() <= 0.2), -0.7 <= ra.f <= 0), end_size = 1, color = color));
                    }

                    // Shooting.
                    if (have_gun_ability && con.shoot.pressed() && !p.shot)
                    {
                        int dir_x = p.facing_left ? -1 : 1;
                        p.shot.emplace();
                        p.shot->pos = p.pos + ivec2(8 * dir_x, -5);
                        p.shot->vel = fvec2(2 * dir_x, 0);
                        Sounds::pew(0.3f);

                        for (int i = 0; i < 20; i++)
                        {
                            fvec3 color(1, ra.f <= 1, 0);
                            par.Add(adjust(Particle{}, s.pos = p.shot->pos + (ra.fvec2.abs() <= 2), damp = 0.01, life = 20 <= ra.i <= 40, size = 1 <= ra.i <= 5, s.vel = p.prev_vel.x * 0.4 + fvec2::dir(ra.f.abs() <= f_pi / 10, ra.f <= 2) with(x *= dir_x), end_size = 1, color = color));
                        }
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

                // Interaction with ghost shots.
                if (controllable)
                {
                    for (Ghost &ghost : time.ghosts)
                    {
                        if (time.time < ghost.time_start)
                            continue;
                        int rel_time = time.time - ghost.time_start;
                        if (rel_time >= int(ghost.states.size()))
                            continue;
                        Player &state = ghost.states[rel_time];
                        if (!state.shot)
                            continue;
                        if ((abs(state.shot->pos - p.pos) < Player::shot_hitbox_halfsize).all())
                        {
                            Sounds::push();

                            p.boost_vel = state.shot->vel.norm() * 4;
                            p.remaining_boost_frames = 190;

                            // Erase this shot from the future.
                            for (std::size_t i = rel_time; i < ghost.states.size(); i++)
                            {
                                if (!ghost.states[i].shot)
                                    break;
                                ghost.states[i].shot.reset();
                            }
                        }
                    }
                }

                // Apply velocity.
                if (controllable)
                {
                    { // Velocity override.
                        if (p.remaining_boost_frames > 0)
                        {
                            p.remaining_boost_frames--;
                            p.vel = p.boost_vel;

                            for (int i = 0; i < 4; i++)
                            {
                                float c = ra.f <= 1;
                                fvec3 color(1 - c * 0.5f, 1 - c * 0.25f, 0);
                                if (ra.boolean())
                                    std::swap(color.x, color.z);
                                par.Add(adjust(Particle{}, s.pos = p.pos + (ra.fvec2.abs() <= fvec2(4, 6)), damp = 0.015, life = 20 <= ra.i <= 60, size = 1 <= ra.i <= 3, s.vel = fvec2::dir(ra.angle(), 0.15f), end_size = 1, color = color, beta = 0));
                            }
                        }
                    }

                    bool hit_something = false;

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
                                hit_something = true;
                                int_vel[i] = 0;
                                if (p.vel[i] * s > 0)
                                    p.vel[i] = 0;
                                if (p.vel_lag[i] * s > 0)
                                    p.vel_lag[i] = 0;
                            }
                        }
                    }

                    if (hit_something)
                        p.remaining_boost_frames = 0;
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

                    if (PickUpAbility(map.ability_timeshift, "Timeshift", FMT("Hold [Z]/[L] to travel back in time\n{} uses remaining", max_timeshifts)))
                        have_timeshift_ability = true;
                    if (PickUpAbility(map.ability_doublejump, "Doublejump", "Press [jump] to jump off of your past copy"))
                        have_doublejump_ability = true;
                    if (PickUpAbility(map.ability_gun, "Fireball", "Press [X]/[K] to shoot\nTouch your past shot to get a boost"))
                        have_gun_ability = true;
                }

                { // Tick the shot.
                    if (p.shot)
                    {
                        p.shot->pos += p.shot->vel;
                        ivec2 round_pos = iround(p.shot->pos);

                        std::set<ivec2> breaking_tiles;
                        bool dies = false;

                        for (ivec2 point : p.shot->hitbox)
                        {
                            ivec2 tile_pos = div_ex(round_pos + point, tile_size);
                            const TileInfo &info = map.at(tile_pos).info();
                            if (info.breakable)
                            {
                                breaking_tiles.insert(tile_pos);
                                dies = true;
                            }
                            else if (info.solid)
                            {
                                dies = true;
                            }
                        }

                        if (!breaking_tiles.empty())
                        {
                            for (ivec2 tile : breaking_tiles)
                            {
                                if (map.cells.pos_in_range(tile))
                                {
                                    map.at(tile).tile = Tile::air;
                                    time.block_breaking_times[tile] = time.time;

                                    for (int i = 0; i < 15; i++)
                                    {
                                        fvec3 color(1, ra.f <= 1, 0);
                                        par.Add(adjust(Particle{}, s.pos = tile * tile_size + (ra.fvec2 <= tile_size), s.acc = fvec2(0,0.01f), damp = 0.01, life = 20 <= ra.i <= 40, size = 1 <= ra.i <= 5, s.vel = fvec2::dir(ra.angle(), ra.f <= 0.23f), end_size = 1, color = color));
                                    }
                                }
                            }
                        }
                        if (dies)
                        {
                            if (breaking_tiles.empty())
                                Sounds::shot_dies(p.shot->pos, 0.4f);
                            else
                                Sounds::shot_breaks_block(p.shot->pos);

                            for (int i = 0; i < 10; i++)
                            {
                                fvec3 color(1, ra.f <= 1, 0);
                                par.Add(adjust(Particle{}, s.pos = p.shot->pos + (ra.fvec2.abs() <= 2), damp = 0.01, life = 20 <= ra.i <= 40, size = 1 <= ra.i <= 5, s.vel = fvec2::dir(ra.f.abs() <= f_pi / 2, ra.f <= 1.3f) with(x *= -sign(p.shot->vel.x)), end_size = 1, color = color));
                            }

                            p.shot.reset();
                        }
                    }
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

                    if (!p.in_prison && time.time % 7 == 0)
                        p.lava_y -= 1;

                    if (have_timeshift_ability)
                        time_since_got_timeshift++;

                    buffered_jump = false;
                }

                { // Determining the current sprite.
                    constexpr int
                    breathing_variant_len = 20,
                    running_variant_len = 8, running_num_variants = 4;

                    constexpr float jumping_variant_speeds[] = {-1, -0.5, 0.5};

                    if (p.remaining_boost_frames > 0)
                    {
                        // Dash.
                        p.anim_state = 2;
                        p.anim_variant = 0;
                    }
                    else if (!p.ground)
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
                bool death_fade = p.dead && p.death_timer >= 60 && (!have_timeshift_ability || time.RemainingShifts() == 0);
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
                DrawAbility(map.ability_doublejump);
                DrawAbility(map.ability_gun);
            }

            { // Shots.
                static const auto &region = texture_atlas.Get("shot.png");
                static const int size = region.size.y;

                if (p.shot)
                {
                    fvec2 rel_pos = p.shot->pos - camera_pos;
                    if ((rel_pos <= screen_size / 2 + 16).all())
                        r.fquad(rel_pos, region.region(ivec2(size * Shot::GetAnimVariant(time.time), 0), ivec2(size))).center().flip_x(p.shot->vel.x < 0);
                }
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
                { // HUD.
                    // Remaining shifts.
                    if (have_timeshift_ability)
                    {
                        int remaining = time.RemainingShifts();
                        float alpha = smoothstep(clamp_max(time_since_got_timeshift / 60.f));

                        Graphics::Text text(Fonts::main, FMT("{}", remaining));
                        for (int i = 0; i < 4; i++)
                            r.itext(ivec2(0, -screen_size.y/2) + ivec2::dir4(i), text).align(ivec2(0,-1)).alpha(alpha).color(fvec3(0));
                        r.itext(ivec2(0, -screen_size.y/2), text).align(ivec2(0,-1)).alpha(alpha).color(remaining == 0 ? fvec3(1, window.Ticks() / 60 % 2, 0) : fvec3(255, 179, 26) / 255);
                    }
                }

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
                            r.itext(ivec2(0, screen_size.y/2) + ivec2::dir4(i), text).align_y(1).alpha(alpha).color(fvec3(0));
                        r.itext(ivec2(0, screen_size.y/2), text).align_y(1).alpha(alpha).color(fvec3(255, 179, 26) / 255);
                    };

                    ShowHint("Hold [Z]/[L] to travel back in time", hint_death_hollback);
                    ShowHint("Press [C]/[J]/[Space] to jump", hint_jump);

                    for (const Hint &hint : hints)
                        ShowHint(hint.message, hint.alpha);
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
