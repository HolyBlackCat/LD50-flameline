#pragma once

#include <exception>
#include <limits>
#include <map>
#include <vector>

#include "game/render.h"
#include "reflection/complete.h"
#include "utils/json.h"
#include "utils/mat.h"
#include "utils/memory_file.h"

class Map
{
  public:
    using index_t = uint16_t;
    using variant_t = uint16_t;
    static constexpr index_t index_none = 0; // We use 0 because Tiled uses it.
    static constexpr variant_t variant_default = 0;

    class TileSheet
    {
        ReflectStruct(TileVariantData, (
            (std::string)(name),
            (ivec2)(tex)(=ivec2(0)),
            (optional)(ivec2)(tex_offset)(=ivec2(0)),
            (optional)(ivec2)(tex_size)(=ivec2(1)),
        ))

        ReflectStruct(TileData, (
            (std::string)(name),
            (std::vector<TileVariantData>)(variants),
            (std::map<std::string, variant_t> variant_indices;),
        ))

        ReflectStruct(Data, (
            (std::string)(name),
            (int)(tile_size)(=1),
            (ivec2)(tex_pos)(=ivec2(0)),
            (std::map<index_t,TileData>)(tiles),
            (std::map<std::string, index_t> tile_indices;),

            // How much tiles to add at window borders. This is needed when images are larger than 1x1 tile.
            (ivec2 overdraw_a = ivec2(0);), // Top-left, zero or negative.
            (ivec2 overdraw_b = ivec2(0);), // Bottom-right, zero or positive.
        ))

        Data data;

        const TileData &FindTile(index_t index) const
        {
            if (auto it = data.tiles.find(index); it != data.tiles.end())
                return it->second;
            else
                Program::Error("No tile ", index, " in sheet `", SheetName(), "`.");
        }
        const TileVariantData &FindVariant(const TileData &tile, variant_t var_index) const
        {
            if (var_index >= tile.variants.size())
                Program::Error("For tilesheet `", SheetName(), "`, variant index ", var_index, " for tile `", tile.name, "` is out of range.");
            return tile.variants[var_index];
        }

      public:
        TileSheet() {}
        TileSheet(const std::string &name, const std::string &string)
        {
            try
            {
                Refl::Interface(data).from_string(string);

                for (auto &tile : data.tiles)
                {
                    // Add tile to the map.
                    data.tile_indices.insert({tile.second.name, tile.first});

                    int var_index = 0;
                    for (const auto &var : tile.second.variants)
                    {
                        // Add variant to the map.
                        tile.second.variant_indices.insert({var.name, var_index++});

                        // Compute overdraw extents.
                        ivec2 a = var.tex_offset;
                        ivec2 b = var.tex_offset + var.tex_size - 1;

                        for (int i = 0; i < 2; i++)
                        {
                            if (a[i] < data.overdraw_a[i])
                                data.overdraw_a[i] = a[i];
                            if (b[i] > data.overdraw_b[i])
                                data.overdraw_b[i] = b[i];
                        }
                    }
                }

            }
            catch (std::exception &e)
            {
                Program::Error("Unable to load tile sheet `", name, "`: ", e.what());
            }
        }
        TileSheet(const std::string &file_name) : TileSheet(file_name, MemoryFile(file_name).construct_string()) {}

        const std::string SheetName() const
        {
            return data.name;
        }
        int TileSize() const
        {
            return data.tile_size;
        }
        ivec2 TexturePos() const
        {
            return data.tex_pos;
        }

        // How much tiles to add at window borders. This is needed when images are larger than 1x1 tile.
        ivec2 OverdrawA() const // Top-left, zero or negative.
        {
            return data.overdraw_a;
        }
        ivec2 OverdrawB() const // Bottom-right, zero or positive.
        {
            return data.overdraw_b;
        }

        const std::string &GetName(index_t index) const
        {
            return FindTile(index).name;
        }
        index_t GetIndexFromName(const std::string &name) const
        {
            if (auto it = data.tile_indices.find(name); it != data.tile_indices.end())
                return it->second;
            else
                Program::Error("Tile sheet `", SheetName(), "` has no tile named `", name, "`.");
        }

        variant_t GetVariantCount(index_t index) const
        {
            return FindTile(index).variants.size();
        }
        variant_t GetVariantFromName(index_t index, const std::string &name) const
        {
            const auto &tile = FindTile(index);
            if (auto it = tile.variant_indices.find(name); it != tile.variant_indices.end())
                return it->second;
            else
                Program::Error("In tile sheet `", SheetName(), "`, tile `", tile.name, "` has no variant named `", name, "`.");
        }

        struct TexInfo
        {
            ivec2 pos = ivec2(0), offset = ivec2(0), size = ivec2(1);
        };
        TexInfo GetTexInfo(index_t index, variant_t var_index) const
        {
            const auto &var = FindVariant(FindTile(index), var_index);
            TexInfo ret;
            ret.pos    = var.tex;
            ret.offset = var.tex_offset;
            ret.size   = var.tex_size;
            return ret;
        }
    };

    struct TileLayerFormat
    {
        std::string name;
        const TileSheet *sheet = 0; // The pointed object should remain alive as long as the map is used.

        TileLayerFormat() {}
        TileLayerFormat(std::string name, const TileSheet *sheet) : name(name), sheet(sheet) {}
    };

    struct Format
    {
        std::vector<TileLayerFormat> tile_layers;
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

        void UnsafeSetIndexName(ivec2 pos, const std::string &name)
        {
            UnsafeSetIndex(pos, Sheet().GetIndexFromName(name));
        }
        void TrySetIndexName(ivec2 pos, const std::string &name)
        {
            TrySetIndex(pos, Sheet().GetIndexFromName(name));
        }

        void UnsafeSetVariantName(ivec2 pos, const std::string &name)
        {
            auto &tile = tiles[pos.x + pos.y * size.x];
            tile.variant = Sheet().GetVariantFromName(tile.index, name);
        }
        void TrySetVariantName(ivec2 pos, const std::string &name)
        {
            if (PosInRange(pos))
                UnsafeSetVariantName(pos, name);
        }

        void ValidateVariantIndices() const
        {
            for (int y = 0; y < size.y; y++)
            for (int x = 0; x < size.x; x++)
            {
                Tile tile = UnsafeGet(ivec2(x, y));
                if (tile.index == index_none)
                    continue;
                variant_t max_variant = Sheet().GetVariantCount(tile.index);
                if (tile.variant >= max_variant)
                    Program::Error("For tile ", tile.index, ", variant index ", tile.variant, " is out of range.");
            }
        }

        void Render(Render &r, ivec2 screen_size, ivec2 cam_pos, float alpha = 1, float beta = 1) const
        {
            int tile_size = Sheet().TileSize();
            ivec2 half_count = (screen_size / 2 - 1) / tile_size + 1;
            ivec2 base_tile = div_ex(cam_pos, tile_size);
            ivec2 overdraw_a = Sheet().OverdrawA();
            ivec2 overdraw_b = Sheet().OverdrawB();
            for (int y = -half_count.y+overdraw_a.y; y <= half_count.y+overdraw_b.y; y++)
            for (int x = -half_count.x+overdraw_a.x; x <= half_count.x+overdraw_b.x; x++)
            {
                ivec2 tile_pos = base_tile + ivec2(x, y);
                Tile tile = ClampGet(tile_pos);
                if (tile.index == index_none)
                    continue;

                auto tex_info = Sheet().GetTexInfo(tile.index, tile.variant);
                r.iquad((tile_pos + tex_info.offset) * tile_size - cam_pos, tex_info.size * tile_size).tex(tex_info.pos * tile_size + Sheet().TexturePos()).alpha(alpha).beta(beta);
            }
        }
    };

  private:
    std::string name;
    std::vector<TileLayer> layers;

  public:
    Map() {}

    Map(const Format &format, const std::string &map_name, const Json::View &data)
    {
        try
        {
            name = map_name;

            auto layers_data = data["layers"];
            int layers_count = layers_data.GetArraySize();

            int last_index = -1;

            for (size_t i = 0; i < format.tile_layers.size(); i++)
            {
                const std::string &layer_name = format.tile_layers[i].name;

                bool found = 0;
                for (int j = 0; j < layers_count; j++)
                {
                    auto this_layer = layers_data[j];

                    if (this_layer["name"].GetString() == layer_name)
                    {
                        if (found)
                            Program::Error("Duplicate layer `", layer_name, "`.");

                        if (i != 0 && j <= last_index)
                            Program::Error("Expected layer `", layer_name, "` to be located after layer `", format.tile_layers[i-1].name, "`.");

                        found = 1;
                        last_index = i;

                        layers.push_back(TileLayer(format.tile_layers[i].sheet, this_layer));

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

    Map(const Format &format, MemoryFile file) : Map(format, file.name(), Json(file.construct_string().c_str(), 64)) {}

    const std::string &Name() const
    {
        return name;
    }

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

    void ValidateVariantIndices()
    {
        for (size_t i = 0; i < layers.size(); i++)
        {
            try
            {
                Layer(i).ValidateVariantIndices();
            }
            catch (std::exception &e)
            {
                Program::Error("In map `", name, "`, layer ", i, ": ", e.what());
            }
        }
    }
};
