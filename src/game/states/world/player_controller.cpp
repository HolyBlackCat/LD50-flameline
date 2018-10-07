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
                    clamp_var(player->vel.x += player->h_control * cfg.player_ground_walking_acc, -cfg.player_speed_cap.x, cfg.player_speed_cap.x);
                }
                else
                {
                    float stopping_acc = player->flying ? cfg.player_flight_stopping_acc :
                                         player->ground ? cfg.player_ground_stopping_acc :
                                                          cfg.player_air_stopping_acc;

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
                if (player->has_second_jump)
                {
                    for (int s = -1; s <= 1; s += 2)
                    {
                        if (player->ground || player->wall_hug_dir == 0)
                            continue;
                        if (!SolidForPlayerAtOffset(world, ivec2(s, cfg.player_wall_slide_hitbox_offset_y)) || !SolidForPlayerAtOffset(world, ivec2(s, 0))) // This is intended, think twice before changing.
                            continue;

                        if (player->vel.y > cfg.player_wall_slide_speed_cap)
                        {
                            player->vel.y = cfg.player_wall_slide_speed_cap;
                            break;
                        }
                    }
                }
            }

            { // Wall jumping
                if ((player->vel.x < 0 ? controls.right : controls.left).down() || player->ground || player->grabbing_ledge)
                    player->flying = 0;

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

                        player->has_second_jump = 0;
                        player->flying = 1;
                        player->vel = cfg.player_wall_jump_vel * ivec2(-s, -1);
                    }
                }
            }

            { // Ledge grabbing
                player->grabbing_ledge = 0;
                if (player->wall_hug_dir != 0)
                {
                    ivec2 collider_pos = player->pos + ivec2(player->wall_hug_dir * (cfg.player_hitbox_size.x/2+1) + (player->wall_hug_dir > 0), -cfg.player_ledge_grab_collider_offest_y);
                    if (world.map.SolidAtPixel(collider_pos) && !world.map.SolidAtPixel(collider_pos.sub_y(1)))
                        player->grabbing_ledge = 1;
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
        }

    }

    void PlayerController::Render(const States::World &world) const
    {
        if (player)
        {
            render.iquad(player->pos - world.camera.Pos(), cfg.player_sprite_size).tex(img_player.pos + cfg.player_sprite_size * ivec2(player->anim_frame, player->anim_state)).center();
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
