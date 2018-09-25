#pragma once

#include "game/state.h"
#include "game/states/world/tile_map.h"

namespace States
{
    struct World : State
    {
        Details::World::TileMap map;

        World()
        {
            map.Load("assets/map.json");
        }

        void Tick() override
        {

        }
        void Render() const override
        {
            map.Render();
        }
    };
}
