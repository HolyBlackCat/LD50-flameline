#pragma once

#include <string>

#include "game/map.h"
#include "utils/mat.h"

namespace States::Details::World
{
    class TileMap
    {
      public:
        inline static Map::TileSheet sheet = Map::TileSheet("assets/tile_sheet.txt");

        inline static Map::Format format = []{
            Map::Format format;
            format.tile_layers.push_back(Map::TileLayerFormat("mid", &sheet));
            format.tile_layers.push_back(Map::TileLayerFormat("front", &sheet));
            return format;
        }();

        Map map;

        TileMap() {}

        void Load(const std::string &name);

        void Render() const;
    };
}
