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
        enum TileInfoHitbox
        {
            hitbox_block, // Hitbox mask no effect.
            hitbox_cover, // 4 lowest bits of hitbox mask determine if 4 strips of size `tile_size * cover_hitbox_width` along tile sides are a part of the hitbox. (3 - right, 2 - down, 1 - left, 0 - up)
        };
        enum TileInfoDrawMode {invis, fancy, flat, pipe, cover};

        struct TileInfo
        {
            std::string name;
            TileInfoHitbox hitbox = hitbox_block;
            bool solid = 0, kills = 0;
            TileInfoDrawMode draw_mode = invis;
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
        static constexpr int cover_hitbox_width = 3; // This affects spikes and other flat tiles.

        struct ExtraData
        {
            Reflect(ExtraData)
            (
                (optional)(fvec3)(light)(=fvec3(1)),
            )
        };

      private:
        std::string name;
        ivec2 size = ivec2(0);
        std::vector<TileStack> tiles;
        std::vector<int> random_values;
        std::vector<int> collision_masks;
        std::vector<int> hitboxes; // Meaning of those integer depends on `TileInfo::hitbox`.
        ExtraData extra_data;

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

        const ExtraData GetExtra() const
        {
            return extra_data;
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

        bool PixelCollidesWithTileHitbox(TileInfoHitbox hitbox_type, int hitbox_mask, ivec2 pixel_pos) const // `pixel_pos` is relative to the tile.
        {
            switch (hitbox_type)
            {
              case hitbox_block:
                return 1;
                break;

              case hitbox_cover:
                {
                    if (hitbox_mask & 0b1000 && pixel_pos.x >= tile_size - cover_hitbox_width)
                        return 1;
                    if (hitbox_mask & 0b0100 && pixel_pos.y >= tile_size - cover_hitbox_width)
                        return 1;
                    if (hitbox_mask & 0b0010 && pixel_pos.x < cover_hitbox_width)
                        return 1;
                    if (hitbox_mask & 0b0001 && pixel_pos.y < cover_hitbox_width)
                        return 1;

                    return 0;
                }
                break;
            }

            return 0;
        }

        bool SolidAtPixel(ivec2 pos) const
        {
            ivec2 tile_pos = div_ex(pos, tile_size);

            int index = ClampGet(tile_pos).mid;
            if (index == -1)
                return 0;

            const TileInfo &info = GetTileInfo(index);
            if (!info.solid)
                return 0;

            return PixelCollidesWithTileHitbox(info.hitbox, hitboxes[tile_pos.x + tile_pos.y * size.x], mod_ex(pos, tile_size));
        }

        bool DangerAtPixel(ivec2 pos) const
        {
            ivec2 tile_pos = div_ex(pos, tile_size);

            int index = ClampGet(tile_pos).mid;
            if (index == -1)
                return 0;

            const TileInfo &info = GetTileInfo(index);
            if (!info.kills)
                return 0;

            return PixelCollidesWithTileHitbox(info.hitbox, hitboxes[tile_pos.x + tile_pos.y * size.x], mod_ex(pos, tile_size));
        }

        void SetColorMatrix() const;
        void ResetColorMatrix() const;
    };
}
