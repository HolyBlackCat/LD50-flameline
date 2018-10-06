#include "program/parachute.h"

#include <utility>
#include <vector>

#include "game/states/world.h"
#include "game/states/world/camera.h"
#include "game/states/world/config.h"
#include "game/meta.h"
#include "game/controls.h"

namespace States::Details::World
{
    static const auto &cfg = *Cfg::World();

    void Camera::Tick(States::World &world)
    {
        // Select target
        if (world.player_controller.PlayerExists())
        {
            target = world.player_controller.GetPlayerPos();
        }

        // Move
        if (first_tick)
        {
            first_tick = 0;
            vel = fvec2(0);
            pos = target;
        }
        else
        {
            fvec2 delta = target - pos;
            float dist = delta.len();
            vel += pow(dist * cfg.camera_dist_factor, cfg.camera_dist_power) / cfg.camera_mass * delta.norm();
            vel *= (1 - cfg.camera_drag);
            pos += vel;
        }
        pixel_pos = iround(pos);
    }
}
