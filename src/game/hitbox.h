#pragma once

#include <vector>

#include "utils/mat.h"

#include "game/states/world/tile_map.h"

namespace Hitboxes
{
    class TileRectHitbox
    {
        ivec2 half_extent = ivec2(0);
        std::vector<ivec2> points;

        using TileMap = States::Details::World::TileMap;

      public:
        TileRectHitbox() {}
        TileRectHitbox(ivec2 half_extent) : half_extent(half_extent)
        {
            ivec2 point_count = (half_extent*2 - 2) / TileMap::tile_size + 2;

            for (int y = 0; y < point_count.y; y++)
            for (int x = 0; x < point_count.x; x++)
            {
                ivec2 pos = ivec2(x,y) * TileMap::tile_size - half_extent;

                for (int i = 0; i < 2; i++)
                    if (pos[i] > half_extent[i] - 1)
                        pos[i] = half_extent[i] - 1;

                points.push_back(pos);
            }
        }

        bool SolidAt(const TileMap &map, ivec2 pos) const
        {
            for (ivec2 point : points)
                if (map.SolidAtPixel(pos + point))
                    return 1;
            return 0;
        }

        bool DangerAt(const TileMap &map, ivec2 pos) const
        {
            for (ivec2 point : points)
                if (map.DangerAtPixel(pos + point))
                    return 1;
            return 0;
        }
    };
}
