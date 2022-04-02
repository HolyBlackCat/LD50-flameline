#include "particles.h"

void ParticleController::Tick(ivec2 camera_pos)
{
    for (Particle &par : particles)
    {
        par.pos += par.vel;
        par.vel += par.acc;
        par.vel *= 1 - par.damp;
        par.current_lifetime++;
    }

    std::erase_if(particles,
        [&](const Particle &par)
        {
            if (par.current_lifetime > par.life) return true;
            if ((abs(par.pos - camera_pos) > screen_size / 2 + 16).any()) return true;
            return false;
        });
}

void ParticleController::Render(ivec2 camera_pos) const
{
    for (const Particle &par : particles)
    {
        float t = par.current_lifetime / float(par.life);

        float size = !par.end_size ? par.size : mix(t, par.size, *par.end_size);
        fvec3 color = !par.end_color ? par.color : mix(t, par.color, *par.end_color);
        float alpha = !par.end_alpha ? par.alpha : mix(t, par.alpha, *par.end_alpha);
        float beta = !par.end_beta ? par.beta : mix(t, par.beta, *par.end_beta);

        r.fquad(par.pos - camera_pos, fvec2(size)).color(color).alpha(alpha).beta(beta).center();
    }
}
