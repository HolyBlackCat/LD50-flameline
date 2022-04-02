#pragma once

#include "game/main.h"

struct Particle
{
    struct State
    {
        fvec2 pos, vel, acc;
        int current_lifetime = 0;
    };
    State s;

    float damp = 0;

    fvec3 color = fvec3(1);
    std::optional<fvec3> end_color;

    float alpha = 1, beta = 1;
    std::optional<float> end_alpha, end_beta;

    float size = 4;
    std::optional<float> end_size;

    int life = 60;

    int state_id = 0;
};

class ParticleController
{
    std::deque<Particle> particles;

    std::vector<std::vector<Particle::State>> states;
    SparseSet<int> state_ids;

public:
    void Add(const Particle &par)
    {
        if (state_ids.IsFull())
        {
            state_ids.Reserve((state_ids.Capacity() + 1) * 2);
            while (int(states.size()) < state_ids.Capacity())
            {
                states.emplace_back();
                states.back().reserve(1024);
            }
        }

        Particle &new_par = particles.emplace_back(par);
        new_par.state_id = state_ids.InsertAny();
        states[new_par.state_id].clear(); // This shouldn't reset capacity, this is intentional.
    }

    void Tick(ivec2 camera_pos);
    void ReverseTick();

    void Render(ivec2 camera_pos) const;
};
