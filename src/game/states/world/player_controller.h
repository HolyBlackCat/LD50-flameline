#pragma once

#include <string>
#include <vector>
#include <utility>

#include "utils/dynamic_storage.h"
#include "utils/mat.h"

namespace States {class World;}

namespace States::Details::World
{
    class PlayerController
    {
        struct Player
        {
            ivec2 pos = ivec2(0);
            fvec2 vel = fvec2(0);
            fvec2 vel_lag = fvec2(0);
            ivec2 vel_int = ivec2(0);

            bool ground = 0;
            int jump_ticks_left = 0;
            int wall_jump_ticks_left = 0;
            fvec2 wall_jump_vel = fvec2(0);
            bool flying_after_wall_jump = 0;
            bool has_second_jump = 0;

            int anim_state = 0;
            int anim_frame = 0;
            int h_control = 0;
        };

        DynStorage<Player> player = nullptr;

      public:
        PlayerController() {}

        void Tick(States::World &world);
        void Render(const States::World &world) const;

        void RespawnPlayer(States::World &world);

        bool PlayerExists() const;
        ivec2 GetPlayerPos() const;

        bool SolidForPlayerAtOffset(const States::World &world, ivec2 offset) const;
    };
}
