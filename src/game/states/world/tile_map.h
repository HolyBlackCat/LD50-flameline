#pragma once

#include <string>
#include <vector>
#include <utility>

#include "reflection/complete.h"

#include "game/texture_atlas.h"
#include "utils/mat.h"

namespace States::Details::World
{
    class TileMap
    {
      public:
        enum TileDrawMode {fancy, cover};

        struct TileInfo
        {
            std::string name;


            TileInfo(const std::string &name) : name(name) {}
        };

        struct TileStack
        {
            Reflect(TileStack)
            (
                (int)(back, mid)(=-1),
            )

            int random = 0;
        };

        static constexpr int tile_size = 12;

      private:
        ivec2 size = ivec2(0);
        std::vector<TileStack> tiles;

      public:
        TileMap() {}

        static const TileInfo &GetTileInfo(int index); // Throws if no such index.

        void Load(const std::string &name);

        void Render() const;

        TileStack &UnsafeAt(ivec2 pos) const
        {
            return const_cast<TileStack &>(std::as_const(*this).UnsafeAt(pos));
        }
        const TileStack &UnsafeAt(ivec2 pos) const
        {
            return tiles[pos.x + pos.y * size.x];
        }
    };
}
