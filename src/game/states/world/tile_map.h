#pragma once

#include <string>

#include "utils/mat.h"

namespace States::Details::World
{
    class TileMap
    {
      public:
        TileMap() {}

        void Load(const std::string &name);

        void Render() const;
    };
}
