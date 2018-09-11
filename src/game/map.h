#pragma once

#include <exception>
#include <limits>
#include <vector>

#include "game/render.h"
#include "utils/json.h"
#include "utils/mat.h"

class Map
{
  public:
    using tile_t = u16vec2;
    using index_t = uint16_t;
    static constexpr tile_t no_tile = tile_t(0xffff);
    static constexpr index_t no_index = 0; // We use 0 because Tiled uses it.

    class TileSheet
    {
        ivec2 tex_pos = ivec2(0);
        ivec2 size = ivec2(0);

      public:
        TileSheet() {}
        TileSheet(ivec2 tex_pos, ivec2 size) : tex_pos(tex_pos), size(size) {}

        ivec2 TexturePos() const
        {
            return tex_pos;
        }
        ivec2 Size() const
        {
            return size;
        }
    };

    class Layer
    {
        ivec2 size = ivec2(0);
        std::vector<tile_t> tiles;
        std::vector<index_t> indices;
        const TileSheet *sheet = 0;

      public:
        Layer() {}

        Layer(std::string map_name, const TileSheet *sheet, const Json::View &data) : sheet(sheet)
        {
            try
            {
                // Stop if this is not a tile layer.
                if (data["type"].GetString() != "tilelayer")
                    Program::Error("Expected a tile layer.");

                // Stop if this layer has non-zero offset. (Tiled manual says this should never happen.)
                if (data["x"].GetInt() != 0 || data["y"].GetInt() != 0)
                    Program::Error("Expected the layer to have zero offset.");

                // Obtain size.
                size.x = data["width"].GetInt();
                size.y = data["height"].GetInt();

                // Check if tile array has a proper number of tiles.
                Json::View data_array = data["data"];
                if (data_array.GetArraySize() != size.prod())
                    Program::Error("Invalid tile count in data array, the file is corrupted.");

                // Read tile indices.
                data_array.ForEachArrayElement([&](const Json::View &elem)
                {
                    int index = elem.GetInt();
                    if (index < std::numeric_limits<index_t>::min() || index > std::numeric_limits<index_t>::max())
                        Program::Error("Tile index out of range.");
                    indices.push_back(index);
                });

                // Allocate tile array.
                tiles.resize(indices.size(), no_tile);
            }
            catch (std::exception &e)
            {
                std::string layer_name;
                try
                {
                    layer_name = data["name"].GetString();
                }
                catch (...)
                {
                    layer_name = "??";
                }

                Program::Error("Unable to load map `", map_name, "`, layer `", layer_name, "`: ", e.what());
            }
        }

        ivec2 Size() const
        {
            return size;
        }

        bool PosInRange(ivec2 pos) const
        {
            return (pos >= 0).all() && (pos < Size()).all();
        }

        tile_t UnsafeGetTile(ivec2 pos) const
        {
            return tiles[pos.x + pos.y * size.x];
        }
        tile_t TryGetTile(ivec2 pos) const
        {
            if (PosInRange(pos))
                return UnsafeGetTile(pos);
            else
                return no_tile;
        }
        tile_t ClampGetTile(ivec2 pos) const
        {
            clamp_var(pos, 0, Size()-1);
            return UnsafeGetTile(pos);
        }

        void UnsafeSetTile(ivec2 pos, tile_t tile)
        {
            tiles[pos.x + pos.y * size.x] = tile;
        }
        void TrySetTile(ivec2 pos, tile_t tile)
        {
            if (PosInRange(pos))
                UnsafeSetTile(pos, tile);
        }

        index_t UnsafeGetIndex(ivec2 pos) const
        {
            return indices[pos.x + pos.y * size.x];
        }
        index_t TryGetIndex(ivec2 pos) const
        {
            if (PosInRange(pos))
                return UnsafeGetIndex(pos);
            else
                return no_index;
        }
        index_t ClampGetIndex(ivec2 pos) const
        {
            clamp_var(pos, 0, Size()-1);
            return UnsafeGetIndex(pos);
        }

        void UnsafeSetIndex(ivec2 pos, index_t index)
        {
            indices[pos.x + pos.y * size.x] = index;
        }
        void TrySetIndex(ivec2 pos, index_t index)
        {
            if (PosInRange(pos))
                UnsafeSetIndex(pos, index);
        }
    };

  private:

  public:
    Map()
    {

    }
};
