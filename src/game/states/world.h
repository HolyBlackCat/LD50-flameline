#pragma once

#include "game/state.h"
#include "game/states/common/fade.h"
#include "game/states/world/camera.h"
#include "game/states/world/particle_controller.h"
#include "game/states/world/player_controller.h"
#include "game/states/world/tile_map.h"

#include "graphics/clear.h"

namespace States
{
    struct World : State
    {
        Details::Common::Fade fade = fvec3(0);
        Details::World::Camera camera;
        Details::World::TileMap map;
        Details::World::PlayerController player_controller;
        Details::World::ParticleController particles;

        World()
        {
            map.Load("assets/maps/tutorial.json");
            player_controller.RespawnPlayer(*this);
        }

        void Tick() override
        {
            player_controller.Tick(*this);
            particles.Tick(*this);
            camera.Tick(*this);
            fade.Tick();
        }

        void Render() const override
        {
            Graphics::Clear();
            map.Render(*this, &Details::World::TileMap::TileStack::back);
            map.Render(*this, &Details::World::TileMap::TileStack::mid);
            map.SetColorMatrix();
            player_controller.Render(*this);
            particles.Render(*this);
            map.ResetColorMatrix();
            fade.Render();
        }
    };
}
