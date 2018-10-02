#include "program/parachute.h"

#include "game/states/world/tile_map.h"

#include "game/meta.h"

namespace States::Details::World
{
    static const auto
        img_test = Atlas().Get("creatures/player/standing.0.png");

    void TileMap::Load(const std::string &name)
    {
        (void)name;
    }

    void TileMap::Render() const
    {
        render.iquad(-screen_size/2, screen_size).color(fvec3(0), fvec3(0), fvec3(0.1), fvec3(0.1));
        render.iquad(mouse.pos(), img_test.size).tex(img_test.pos).center().rotate(sin(metronome.ticks / 100.));
    }
}
