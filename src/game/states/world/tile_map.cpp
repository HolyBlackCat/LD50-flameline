#include "program/parachute.h"

#include <exception>
#include <utility>
#include <vector>

#include "game/states/world/tile_map.h"
#include "game/meta.h"
#include "utils/json.h"

namespace States::Details::World
{
    static const std::vector<TileMap::TileInfo> tile_list
    {
        { "moss_stone"  , TileMap::fancy }, // 0
        { "grass_cover" , TileMap::cover }, // 1
        { "stone"       , TileMap::fancy }, // 2
    };

    const TileMap::TileInfo &TileMap::GetTileInfo(int index)
    {
        if (index < 0 || index >= int(tile_list.size()))
            Program::Error("Tile index ", index, " is out of range.");
        return tile_list[index];
    }

    void TileMap::Load(const std::string &name)
    {
        try
        {
            // Open map file.
            Json json(MemoryFile(name).construct_string().c_str(), 64);
            Json::View view = json.GetView();
            Json::View view_layer_list = view["layers"];

            // Get size.
            size = ivec2(view["width"].GetInt(), view["height"].GetInt());
            tiles.resize(size.prod());

            // Generate random numbers.
            for (auto &tile_stack : tiles)
                tile_stack.random = random.integer() <= 0xffff;


            using tile_refl = Refl::Interface<TileStack>;
            tile_refl::for_each_field([&](auto index)
            {
                constexpr int layer_index = index.value;
                std::string layer_name = tile_refl::field_name(i);
                Json::View view_layer = view_layer_list[layer_index];

                // Stop if layer is named incorrectly.
                if (view_layer["name"].GetString() != layer_name)
                    Program::Error("Expected layer ", layer_index, "` to be named `", layer_name, "`.");

                // Stop if it's not a tile layer.
                if (view_layer["type"].GetString() != "tilelayer")
                    Program::Error("Expected layer `", layer_name, "` to be a tile layer.");

                Json::View view_data = view_layer["data"];

                // Stop if data array size is incorrect.
                if (view_data.GetArraySize() != size.prod())
                    Program::Error("Incorrect format: Layer `", layer_name, "` data array contains an incorrect number of entries.");

                for (int y = 0; y < size.y; y++)
                for (int x = 0; x < size.x; x++)
                {

                }

            });

        }
        catch (std::exception &e)
        {
            Program::Error("Unable to load map `", name, "`:\n", e.what());
        }
    }

    void TileMap::Render() const
    {
        render.iquad(-screen_size/2, screen_size).color(fvec3(0), fvec3(0), fvec3(0.1), fvec3(0.1));
        //render.iquad(mouse.pos(), tile_list[0].img[0][0].size).tex(tile_list[0].img[0][0].pos).center().rotate(sin(metronome.ticks / 100.));
    }
}
