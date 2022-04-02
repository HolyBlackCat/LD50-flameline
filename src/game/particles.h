#pragma once

#include "game/main.h"

struct Particle
{
    fvec2 pos, vel, acc;
    float damp = 0;

    fvec3 color = fvec3(1);
    std::optional<fvec3> end_color;

    float alpha = 1, beta = 1;
    std::optional<float> end_alpha, end_beta;

    float size = 4;
    std::optional<float> end_size;

    int life = 60;
    int current_lifetime = 0;
};

struct ParticleController
{
    std::deque<Particle> particles;

    void Tick(ivec2 camera_pos);
    void Render(ivec2 camera_pos) const;
};
