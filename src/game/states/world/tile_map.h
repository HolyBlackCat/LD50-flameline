#pragma once

#include <map>
#include <string>
#include <vector>
#include <utility>

#include "reflection/complete.h"

#include "utils/mat.h"

namespace States {class World;}

namespace States::Details::World
{
    class TileMap
    {
      public:
        enum Solidity {solid, nonsolid};
        enum TileDrawMode {invis, fancy, cover};

        struct TileInfo
        {
            std::string name;
            Solidity solidity;
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
        std::string name;
        ivec2 size = ivec2(0);
        std::vector<TileStack> tiles;
        std::vector<int> random_values;

        using points_t = std::vector<ivec2>;
        using point_list_t = std::map<std::string, std::vector<ivec2>>;
        point_list_t points;

      public:
        TileMap() {}

        static const TileInfo &GetTileInfo(int index); // Throws if no such index.

        void Load(const std::string &name);

        void Render(const States::World &world, int TileStack::*layer) const;

        const std::string Name() const
        {
            return name;
        }

        const point_list_t &GetPoints() const
        {
            return points;
        }

        const points_t *GetPoints(std::string name) const
        {
            if (auto it = points.find(name); it != points.end())
                return &it->second;
            else
                return 0;
        }

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

        TileStack TryGet(ivec2 pos) const
        {
            if (!TilePosInRange(pos))
                return {};
            return UnsafeAt(pos);
        }

        bool SolidAtPixel(ivec2 pos) const
        {
            int index = ClampGet(div_ex(pos, tile_size)).mid;
            if (index == -1)
                return 0;
            return GetTileInfo(index).solidity == solid;
        }
    };
}
