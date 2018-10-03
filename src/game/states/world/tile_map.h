#pragma once

#include <string>
#include <vector>

#include "reflection/complete.h"

#include "game/texture_atlas.h"
#include "utils/mat.h"

namespace States::Details::World
{
    class TileMap
    {
      public:
        struct TileImages
        {
            Reflect(TileImages)
            (
                (TextureAtlas::ImageList)
                (
                    mid,
                    up, down, left, right,
                    up_left, up_right, down_left, down_right, // outer corners
                    left_up, right_up, left_down, right_down // inner corners
                ),
            )

            TileImages() {}
            TileImages(const std::string &name);

            TextureAtlas::ImageList &operator[](int index)
            {
                return const_cast<TextureAtlas::ImageList &>(std::as_const(*this)[index]);
            }
            const TextureAtlas::ImageList &operator[](int index) const
            {
                constexpr int field_count = Refl::Interface<TileImages>().field_count();
                static_assert(sizeof(TileImages) == field_count * sizeof(TextureAtlas::ImageList));
                if (index < 0 || index >= field_count)
                    Program::Error("Tile variant index is out of range.");
                return *reinterpret_cast<const TextureAtlas::ImageList *>(reinterpret_cast<const char *>(this) + index * sizeof(TextureAtlas::ImageList));
            }
        };

        struct TileInfo
        {
            std::string name;
            TileImages img;

            TileInfo(const std::string &name) : name(name), img("tiles/" + name) {}
        };

        struct Tile
        {
            int index = -1;
            int variant = 0;
            int random = 0;
        };
        struct TileStack
        {
            Reflect(TileStack)
            (
                (Tile)(back, mid),
            )
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
    };
}
