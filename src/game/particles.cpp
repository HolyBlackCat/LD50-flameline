#include "particles.h"

void ParticleController::Tick(ivec2 camera_pos)
{
    for (Particle &par : particles)
    {
        par.s.pos += par.s.vel;
        par.s.vel += par.s.acc;
        par.s.vel *= 1 - par.damp;
        par.s.current_lifetime++;

        if (saves_timelines)
            states[par.state_id].push_back(par.s);
    }

    std::erase_if(particles, [&](const Particle &par)
    {
        bool erase = false;
        if (par.s.current_lifetime > par.life)
            erase = true;
        if ((abs(par.s.pos - camera_pos) > screen_size / 2 + 16).any())
            erase = true;

        if (erase && saves_timelines)
            state_ids.EraseUnordered(par.state_id);
        return erase;
    });

    ASSERT(!saves_timelines || int(particles.size()) == state_ids.ElemCount());
}

void ParticleController::ReverseTick()
{
    std::erase_if(particles, [&](const Particle &par)
    {
        bool erase = states[par.state_id].empty();
        if (erase && saves_timelines)
            state_ids.EraseUnordered(par.state_id);
        return erase;
    });

    for (Particle &par : particles)
    {
        auto &state_vec = states[par.state_id];
        if (state_vec.empty())
            continue; // This shouldn't happen.
        par.s = state_vec.back();
        state_vec.pop_back();
    }

    ASSERT(!saves_timelines || int(particles.size()) == state_ids.ElemCount());
}

void ParticleController::Render(ivec2 camera_pos) const
{
    for (const Particle &par : particles)
    {
        float t = par.s.current_lifetime / float(par.life);

        float size = !par.end_size ? par.size : mix(t, par.size, *par.end_size);
        fvec3 color = !par.end_color ? par.color : mix(t, par.color, *par.end_color);
        float alpha = !par.end_alpha ? par.alpha : mix(t, par.alpha, *par.end_alpha);
        float beta = !par.end_beta ? par.beta : mix(t, par.beta, *par.end_beta);

        r.fquad(par.s.pos - camera_pos, fvec2(size)).color(color).alpha(alpha).beta(beta).center();
    }
}
