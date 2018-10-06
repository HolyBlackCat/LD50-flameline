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
        (float)(player_ground_walking_acc)(=2),
        (float)(player_ground_stopping_acc)(=2),
        (float)(player_air_stopping_acc)(=1),
        (float)(player_flight_stopping_acc)(=1),
        (float)(player_jump_speed_cap)(=2),
        (float)(player_jump_speed_begin)(=2),
        (float)(player_jump_speed_end)(=1),
        (int)(player_jump_ticks)(=15),
        (fvec2)(player_wall_jump_speed)(=fvec2(2,2)),
        (int)(player_wall_jump_ticks)(=10),
        (float)(player_wall_glide_speed_cap)(=0.8),
    ))

    inline const auto &World()
    {
        static Config<ObjWorld> ret("assets/config/world.refl");
        return ret;
    }
}

