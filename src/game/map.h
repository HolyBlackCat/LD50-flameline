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
    using index_t = uint16_t;
    using variant_t = uint16_t;
    static constexpr index_t index_none = 0; // We use 0 because Tiled uses it.
    static constexpr variant_t variant_default = 0;

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

    struct Format
    {
        std::vector<std::string> tile_layers;
    };

    struct Tile
    {
        index_t index = index_none;
        variant_t variant = variant_default;

        Tile(index_t index = index_none, variant_t variant = variant_default) : index(index), variant(variant) {}
    };

    class TileLayer
    {
        ivec2 size = ivec2(0);
        std::vector<Tile> tiles;

        const TileSheet *sheet = 0;

      public:
        TileLayer() {}

        TileLayer(const TileSheet *sheet, const Json::View &data) : sheet(sheet) // Pointer to sheet is saved for future use.
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
                tiles.reserve(size.prod());
                data_array.ForEachArrayElement([&](const Json::View &elem)
                {
                    int index = elem.GetInt();
                    if (index < std::numeric_limits<index_t>::min() || index > std::numeric_limits<index_t>::max())
                        Program::Error("Tile index out of range.");
                    tiles.push_back(Tile(index));
                });
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

                Program::Error("Unable to load layer `", layer_name, "`: ", e.what());
            }
        }

        explicit operator bool() const
        {
            return tiles.size() > 0;
        }

        ivec2 Size() const
        {
            return size;
        }

        const TileSheet &Sheet() const
        {
            return *sheet;
        }

        bool PosInRange(ivec2 pos) const
        {
            return (pos >= 0).all() && (pos < Size()).all();
        }

        Tile UnsafeGet(ivec2 pos) const
        {
            return tiles[pos.x + pos.y * size.x];
        }
        Tile ClampGet(ivec2 pos) const
        {
            clamp_var(pos, 0, Size()-1);
            return UnsafeGet(pos);
        }
        Tile TryGet(ivec2 pos) const
        {
            if (PosInRange(pos))
                return UnsafeGet(pos);
            else
                return Tile();
        }

        void UnsafeSet(ivec2 pos, Tile tile)
        {
            tiles[pos.x + pos.y * size.x] = tile;
        }
        void TrySet(ivec2 pos, Tile tile)
        {
            if (PosInRange(pos))
                UnsafeSet(pos, tile);
        }

        void UnsafeSetIndex(ivec2 pos, index_t index)
        {
            tiles[pos.x + pos.y * size.x].index = index;
        }
        void TrySetIndex(ivec2 pos, index_t index)
        {
            if (PosInRange(pos))
                UnsafeSetIndex(pos, index);
        }

        void UnsafeSetVariant(ivec2 pos, variant_t variant)
        {
            tiles[pos.x + pos.y * size.x].variant = variant;
        }
        void TrySetVariant(ivec2 pos, variant_t variant)
        {
            if (PosInRange(pos))
                UnsafeSetVariant(pos, variant);
        }
    };

  private:
    std::vector<TileLayer> layers;

  public:
    Map() {}

    Map(const Format &format, const TileSheet *sheet, const std::string &map_name, const Json::View &data) // Pointer to sheet is saved for future use.
    {
        try
        {
            auto layers_data = data["layers"];
            int layers_count = layers_data.GetArraySize();

            int last_index = -1;

            for (size_t i = 0; i < format.tile_layers.size(); i++)
            {
                const std::string &layer_name = format.tile_layers[i];

                bool found = 0;
                for (int j = 0; j < layers_count; j++)
                {
                    auto this_layer = layers_data[j];

                    if (this_layer["name"].GetString() == layer_name)
                    {
                        if (found)
                            Program::Error("Duplicate layer `", layer_name, "`.");

                        if (i != 0 && j <= last_index)
                            Program::Error("Expected layer `", layer_name, "` to be located after layer `", format.tile_layers[i-1], "`.");

                        found = 1;
                        last_index = i;

                        layers.push_back(TileLayer(sheet, this_layer));

                        // No `break` here because we want to check other layers for duplicates.
                    }
                }

                if (!found)
                    Program::Error("No layer named `", layer_name, "`.");
            }
        }
        catch (std::exception &e)
        {
            Program::Error("Unable to load map `", map_name, "`: ", e.what());
        }
    }

    Map(const Format &format, const TileSheet *sheet, MemoryFile file) : Map(format, sheet, file.name(), Json(file.construct_string().c_str(), 64)) {}

    int LayerCount() const
    {
        return layers.size();
    }

    TileLayer &Layer(int index)
    {
        if (index < 0 || size_t(index) >= layers.size())
            Program::Error("Map layer index is out of range.");
        return layers[index];
    }
    const TileLayer &Layer(int index) const
    {
        if (index < 0 || size_t(index) >= layers.size())
            Program::Error("Map layer index is out of range.");
        return layers[index];
    }
};
