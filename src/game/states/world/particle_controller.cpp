#include "program/parachute.h"

#include <algorithm>

#include "game/states/world.h"
#include "game/states/world/config.h"
#include "game/states/world/camera.h"
#include "game/states/world/particle_controller.h"
#include "game/meta.h"

namespace States::Details::World
{
    void ParticleController::Tick(States::World &world)
    {
        (void)world;
        for (auto &it : particles)
        {
            // Increment time.
            it.time++;

            // Apply velocity matrix.
            it.vel = it.vel_mat * it.vel;

            // Calculate position change.
            fvec2 vel_sum = it.vel + it.vel_lag;
            ivec2 vel_int = iround(vel_sum);
            it.vel_lag = vel_sum - vel_int;

            // Change position.
            it.pos += vel_int;
        }

        // Remove dead particles.
        particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle &p){return p.time >= p.max_time;}), particles.end());
    }

    void ParticleController::Render(const States::World &world) const
    {
        (void)world;
//        for (const auto &it : particles)
//            render.iquad(it.pos, );
    }
}
