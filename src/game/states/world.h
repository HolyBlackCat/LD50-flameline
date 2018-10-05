#pragma once

#include "game/state.h"
#include "game/states/common/fade.h"
#include "game/states/world/camera.h"
#include "game/states/world/tile_map.h"

#include "graphics/clear.h"

namespace States
{
    struct World : State
    {
        Details::Common::Fade fade = fvec3(0);
        Details::World::Camera camera;
        Details::World::TileMap map;

        World()
        {
            map.Load("assets/maps/tutorial.json");
        }

        void Tick() override
        {
            camera.Tick();
            fade.Tick();
        }
        void Render() const override
        {
            Graphics::Clear();
            map.Render(*this, &Details::World::TileMap::TileStack::back);
            map.Render(*this, &Details::World::TileMap::TileStack::mid);
            fade.Render();
        }
    };
}
