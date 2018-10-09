#include "program/parachute.h"

#include "game/states/world.h"
#include "game/states/world/config.h"
#include "game/states/world/player_controller.h"
#include "game/meta.h"
#include "game/controls.h"
#include "game/hitbox.h"

namespace States::Details::World
{
    static const auto &cfg = *Cfg::World();

    static const auto
        img_player = Atlas().Get("creatures/player.png");

    static const Hitboxes::TileRectHitbox player_tile_hitbox(cfg.player_hitbox_size/2);

    void PlayerController::Tick(States::World &world)
    {
        if (player)
        {
            { // Checking for ground
                player->ground = SolidForPlayerAtOffset(world, ivec2(0,1));
            }

            { // Walking
                player->h_control = controls.right.down() - controls.left.down();

                if (player->h_control)
                {
                    float acc = player->ground ? cfg.player_ground_move_acc : cfg.player_air_move_acc;
                    player->facing_left = player->h_control < 0;
                    clamp_var(player->vel.x += player->h_control * acc, -cfg.player_speed_cap.x, cfg.player_speed_cap.x);
                }
                else
                {
                    float stopping_acc = player->ground ? cfg.player_ground_stopping_acc : cfg.player_air_stopping_acc;

                    if (abs(player->vel.x) < stopping_acc)
                    {
                        player->vel.x = 0;
                        player->vel_lag.x = 0;
                    }
                    else
                    {
                        player->vel.x -= sign(player->vel.x) * stopping_acc;
                    }
                }
            }

            { // Jumping
                bool can_jump = player->ground || player->grabbing_ledge;

                if (can_jump)
                    player->has_second_jump = 1;

                if (controls.jump_ok.pressed() && can_jump)
                    player->jump_ticks_left = (!player->grabbing_ledge ? cfg.player_jump_ticks : cfg.player_ledge_jump_ticks);

                if (controls.jump_ok.released() || SolidForPlayerAtOffset(world, ivec2(0,-1)))
                    player->jump_ticks_left = 0;

                if (player->jump_ticks_left > 0)
                {
                    player->jump_ticks_left--;

                    float t = ipow(player->jump_ticks_left / float(cfg.player_jump_ticks), 2);
                    player->vel.y = -min(cfg.player_jump_speed_begin * t + cfg.player_jump_speed_end * (1-t), cfg.player_jump_speed_cap);
                }
            }

            { // Test for wall hugging
                player->wall_hug_dir = 0;
                for (int s = -1; s <= 1; s += 2)
                {
                    if ((s < 0 ? controls.left : controls.right).down() && SolidForPlayerAtOffset(world, ivec2(s, 0)))
                    {
                        player->wall_hug_dir = s;
                        break;
                    }
                }
            }

            { // Wall sliding
                // This should be above ledge grabbing because that can prevent sliding.

                player->wall_sliding = 0;

                if (player->has_second_jump && player->vel.y >= 0)
                {
                    for (int s = -1; s <= 1; s += 2)
                    {
                        if (player->ground || player->wall_hug_dir == 0)
                            continue;

                        ivec2 collider_pos = player->pos + ivec2(s * (cfg.player_hitbox_size.x/2) - (s < 0), cfg.player_wall_slide_collider_offset_y);
                        if (!world.map.SolidAtPixel(collider_pos))
                            continue;

                        player->wall_sliding = 1;
                        if (player->vel.y > cfg.player_wall_slide_speed_cap)
                        {
                            player->vel.y = cfg.player_wall_slide_speed_cap;
                            break;
                        }
                    }
                }
            }

            { // Wall jumping
                if (player->has_second_jump && !player->grabbing_ledge)
                {
                    for (int s = -1; s <= 1; s += 2)
                    {
                        if (player->ground)
                            continue;
                        if (!SolidForPlayerAtOffset(world, ivec2(s, 0)))
                            continue;
                        if (!controls.jump_ok.pressed())
                            continue;

                        player->wall_sliding = 0;
                        player->has_second_jump = 0;
                        player->vel = cfg.player_wall_jump_vel * ivec2(-s, -1);
                        player->facing_left = s > 0;
                    }
                }
            }

            { // Ledge grabbing
                // This should be below wall sliding because it can disable sliding.

                player->grabbing_ledge = 0;
                player->grabbing_ledge_partial = 0;

                if (player->wall_hug_dir != 0 && player->vel.y >= 0)
                {
                    ivec2 collider_pos = player->pos + ivec2(player->wall_hug_dir * (cfg.player_hitbox_size.x/2) - (player->wall_hug_dir < 0), cfg.player_ledge_grab_collider_offest_y);

                    if (!world.map.SolidAtPixel(collider_pos.sub_y(1)))
                    {
                        for (int i = 0; i < cfg.player_ledge_grab_collider_height; i++)
                        {
                            if (world.map.SolidAtPixel(collider_pos.add_y(i)))
                            {
                                if (i == 0)
                                {
                                    player->grabbing_ledge = 1;
                                }
                                else
                                {
                                    player->grabbing_ledge_partial = 1;
                                    if (player->vel.y > cfg.player_wall_slide_speed_cap)
                                        player->vel.y = cfg.player_wall_slide_speed_cap;
                                }

                                player->wall_sliding = 0;

                                break;
                            }
                        }
                    }
                }
            }

            { // Gravity
                player->vel.y += cfg.global_gravity;
            }

            { // Speed calculation
                { // Imposing speed limit
                    for (int i = 0; i < 2; i++)
                    for (int s = -1; s <= 1; s += 2)
                    {
                        if (player->vel[i] * s > cfg.player_speed_cap[i])
                        {
                            player->vel[i] = cfg.player_speed_cap[i] * s;
                            if (player->vel_lag[i] * s > 0)
                                player->vel_lag[i] = 0;
                        }
                    }
                }

                { // Prevent falling when grabbing a ledge
                    if (player->grabbing_ledge)
                    {
                        if (player->vel.y > 0)
                        {
                            player->vel.y = 0;
                            if (player->vel_lag.y > 0)
                                player->vel_lag.y = 0;
                        }
                    }
                }

                fvec2 vel_sum = player->vel + player->vel_lag;
                player->vel_int = iround(vel_sum);
                player->vel_lag = vel_sum - player->vel_int;
            }

            { // Movement
                ivec2 delta = player->vel_int;
                while (delta != 0)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        if (delta[i] == 0)
                            continue;

                        int s = sign(delta[i]);
                        ivec2 offset(0);
                        offset[i] = s;

                        if (!SolidForPlayerAtOffset(world, offset))
                        {
                            player->pos += offset;
                        }
                        if (SolidForPlayerAtOffset(world, offset))
                        {
                            if (player->vel[i] * s > 0)
                            {
                                player->vel[i] = 0;
                                if (player->vel_lag[i] * s > 0)
                                    player->vel_lag[i] = 0;
                            }
                        }

                        delta[i] -= s;
                    }
                }
            }

            { // Animation
                if (player->ground && player->h_control != 0)
                    player->time_running++;
                else
                    player->time_running = 0;

                if (player->wall_sliding) // Wall sliding.
                {
                    player->anim_state = 1;
                    player->anim_frame = 0;
                }
                else if (player->grabbing_ledge || player->grabbing_ledge_partial) // Grabbind ledge.
                {
                    player->anim_state = 2;
                    player->anim_frame = 0;
                }
                else if (player->time_running != 0) // Running.
                {
                    player->anim_state = 3;

                    int uncapped_frame_count = player->time_running;
                    if (uncapped_frame_count <= cfg.player_anim_running_initial_frame_ticks)
                        player->anim_frame = 0;
                    else
                        player->anim_frame = (uncapped_frame_count - cfg.player_anim_running_initial_frame_ticks) / cfg.player_anim_running_frame_len % cfg.player_anim_running_frames;
                }
                else if (!player->ground && abs(player->vel.x) < cfg.player_anim_jumping_far_horizontal_speed_threshold) // Jumping.
                {
                    player->anim_state = 4;

                    if (player->vel.y < cfg.player_anim_jumping_ascent_frame_speed_threshold)
                        player->anim_frame = 0;
                    else if (player->vel.y < cfg.player_anim_jumping_peak_frame_speed_threshold)
                        player->anim_frame = 1;
                    else if (player->vel.y < cfg.player_anim_jumping_descent_frame_speed_threshold)
                        player->anim_frame = 2;
                    else
                        player->anim_frame = 3;
                }
                else if (!player->ground) // Jumping horizontally.
                {
                    player->anim_state = 5;

                    if (player->vel.y < cfg.player_anim_jumping_far_ascent_frame_speed_threshold)
                        player->anim_frame = 0;
                    else if (player->vel.y < cfg.player_anim_jumping_far_descent_frame_speed_threshold)
                        player->anim_frame = 1;
                    else
                        player->anim_frame = 2;
                }
                else // Standing.
                {
                    player->anim_state = 0;
                    player->anim_frame = 0;
                }
            }
        }
    }

    void PlayerController::Render(const States::World &world) const
    {
        if (player)
        {
            render.iquad(player->pos - world.camera.Pos(), cfg.player_sprite_size).tex(img_player.pos + cfg.player_sprite_size * ivec2(player->anim_frame, player->anim_state)).center().flip_x(player->facing_left);
        }
    }

    void PlayerController::RespawnPlayer(States::World &world)
    {
        player = decltype(player)();

        auto spawn_points = world.map.GetPoints("player_spawn");

        // Stop if no spawn point.
        if (!spawn_points)
            Program::Error("No spawn point set for map `", world.map.Name(), "`.");

        // Stop if too many spawn points.
        if (spawn_points->size() != 1)
            Program::Error("Too many spawn points for map `", world.map.Name(), "`.");

        player->pos = spawn_points->front().sub_y(4);
    }

    bool PlayerController::PlayerExists() const
    {
        return bool(player);
    }

    ivec2 PlayerController::GetPlayerPos() const
    {
        if (!player)
            return ivec2(0);
        return player->pos;
    }

    bool PlayerController::SolidForPlayerAtOffset(const States::World &world, ivec2 offset) const
    {
        if (!player)
            return 0;
        return player_tile_hitbox.CollidesAt(world.map, player->pos + offset);
    }
}
