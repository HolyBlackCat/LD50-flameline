#pragma once

#include <string>
#include <vector>
#include <utility>

#include "reflection/complete.h"

#include "game/texture_atlas.h"
#include "utils/mat.h"

namespace States {class World;}

namespace States::Details::World
{
    class TileMap
    {
      public:
        enum TileDrawMode {invis, fancy, cover};

        struct TileInfo
        {
            std::string name;
            TileDrawMode draw_mode = invis;
            int image_index = 0;
        };

        struct TileStack
        {
            Reflect(TileStack)
            (
                (int)(back, mid)(=-1),
            )
        };

        static constexpr int tile_size = 12;

      private:
        ivec2 size = ivec2(0);
        std::vector<TileStack> tiles;
        std::vector<int> random_values;

      public:
        TileMap() {}

        static const TileInfo &GetTileInfo(int index); // Throws if no such index.

        void Load(const std::string &name);

        void Render(const States::World &world, int TileStack::*layer) const;

        bool TilePosInRange(ivec2 pos) const
        {
            return (pos >= 0).all() && (pos < size).all();
        }

        int GetRandom(ivec2 pos) const
        {
            pos = mod_ex(pos, size);
            return random_values[pos.x + size.x * pos.y];
        }

        TileStack &UnsafeAt(ivec2 pos)
        {
            return const_cast<TileStack &>(std::as_const(*this).UnsafeAt(pos));
        }
        const TileStack &UnsafeAt(ivec2 pos) const
        {
            return tiles[pos.x + pos.y * size.x];
        }

        const TileStack &ClampGet(ivec2 pos) const
        {
            clamp_var(pos, 0, size-1);
            return UnsafeAt(pos);
        }
    };
}
