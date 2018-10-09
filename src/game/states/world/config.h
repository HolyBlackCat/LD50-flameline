#pragma once

#include "utils/config.h"

namespace Cfg
{
    ReflectStruct(ObjWorld,(
        (float)(global_gravity)(=0.01),
        (float)(camera_drag)(=0.05),
        (float)(camera_mass)(=1),
        (float)(camera_dist_factor)(=0.005),
        (float)(camera_dist_power)(=2),
        (ivec2)(player_sprite_size)(=ivec2(32)),
        (ivec2)(player_hitbox_size)(=ivec2(32)),
        (fvec2)(player_speed_cap)(=ivec2(8)),
        (float)(player_ground_move_acc)(=2),
        (float)(player_ground_stopping_acc)(=2),
        (float)(player_air_move_acc)(=1),
        (float)(player_air_stopping_acc)(=1),
        (float)(player_jump_speed_cap)(=2),
        (float)(player_jump_speed_begin)(=2),
        (float)(player_jump_speed_end)(=1),
        (int)(player_jump_ticks)(=15),
        (int)(player_ledge_jump_ticks)(=15),
        (fvec2)(player_wall_jump_vel)(=fvec2(2,2)),
        (float)(player_wall_slide_speed_cap)(=0.8),
        (int)(player_wall_slide_collider_offset_y)(=-2),
        (int)(player_ledge_grab_collider_offest_y)(=-4), // This offsets the upper edge of the collider.
        (int)(player_ledge_grab_collider_height)(=5), // Should be 1 or more. Also should be at least as large as y speed cap + 1, otherwise you can miss a grab if you're falling too fast.
        (int)(player_anim_running_frame_len)(=12),
        (int)(player_anim_running_frames)(=6),
        (int)(player_anim_running_initial_frame_ticks)(=6),
        (float)(player_anim_jumping_ascent_frame_speed_threshold)(=-2),
        (float)(player_anim_jumping_peak_frame_speed_threshold)(=1),
        (float)(player_anim_jumping_descent_frame_speed_threshold)(=2),
        (float)(player_anim_jumping_far_horizontal_speed_threshold)(=1),
        (float)(player_anim_jumping_far_ascent_frame_speed_threshold)(=0),
        (float)(player_anim_jumping_far_descent_frame_speed_threshold)(=-2),
        (int)(player_camera_offset_y)(=-32),
    ))

    inline const auto &World()
    {
        static Config<ObjWorld> ret("assets/config/world.refl");
        return ret;
    }
}

