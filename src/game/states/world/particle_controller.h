#pragma once

#include <deque>

#include "utils/mat.h"

namespace States {class World;}

namespace States::Details::World
{
    class ParticleController
    {
        struct Particle
        {
            ivec2 pos = ivec2(0);
            fvec2 vel = fvec2(0), vel_lag = fvec2(0);
            fmat2 vel_mat = fmat2();

            int time = 0;
            int max_time = 0;

            fvec3 color = fvec3(1), color_end = fvec3(1);
            float alpha = 1, alpha_end = 1;
            float beta = 1, beta_end = 1;
            float size = 8, size_end = 0;
        };

        std::deque<Particle> particles;

      public:
        ParticleController() {}

        void Tick(States::World &world);
        void Render(const States::World &world) const;
    };
}
