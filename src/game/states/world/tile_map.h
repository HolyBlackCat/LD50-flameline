#pragma once

#include <string>

#include "game/map.h"
#include "utils/mat.h"

namespace States::Details::World
{
    class TileMap
    {
      public:
        static Map::TileSheet &Sheet()
        {
            static Map::TileSheet ret("assets/tile_sheet.txt");
            return ret;
        }

        inline static Map::Format format = []{
            Map::Format format;
            format.tile_layers.push_back(Map::TileLayerFormat("mid", &Sheet()));
            format.tile_layers.push_back(Map::TileLayerFormat("front", &Sheet()));
            return format;
        }();

        Map map;

        TileMap() {}

        void Load(const std::string &name);

        void Render() const;
    };
}
