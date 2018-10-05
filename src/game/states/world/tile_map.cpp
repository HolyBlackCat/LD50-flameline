#include "program/parachute.h"

#include <exception>
#include <utility>
#include <vector>

#include "game/states/world.h"
#include "game/states/world/tile_map.h"
#include "game/meta.h"
#include "utils/json.h"

namespace States::Details::World
{
    static const auto img_tiles = Atlas().Get("tiles.png");

    static const std::vector<TileMap::TileInfo> tile_list
    {
        { "moss_stone"  , TileMap::fancy , 0}, // 0
        { "grass_cover" , TileMap::cover , 0}, // 1
        { "metal"       , TileMap::fancy , 1}, // 2
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
            random_values.resize(size.prod());

            // Generate random numbers.
            for (int &random_value : random_values)
                random_value = random.integer() <= 0xffff;


            using tile_refl = Refl::Interface<TileStack>;
            tile_refl::for_each_field([&](auto index)
            {
                constexpr int layer_index = index.value;
                std::string layer_name = tile_refl::field_name(index);
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
                    Refl::Interface(UnsafeAt(ivec2(x,y))).template field_value<layer_index>() = view_data[x + size.x * y].GetInt() - 1; // Tiled uses 1-based tile indexing, and we want 0-based.

            });

        }
        catch (std::exception &e)
        {
            Program::Error("Unable to load map `", name, "`:\n", e.what());
        }
    }

    void TileMap::Render(const States::World &world, int TileStack::*layer) const
    {
        ivec2 count = div_ex(screen_size/2, tile_size) * 2 + 2;
        ivec2 base_tile = div_ex(world.camera.Pos() - screen_size/2, tile_size);

        for (int y = 0; y < count.y; y++)
        for (int x = 0; x < count.x; x++)
        {
            int index = ClampGet(base_tile + ivec2(x,y)).*layer;
            if (index == -1)
                continue;

            const auto &info = GetTileInfo(index);
            int r = GetRandom(base_tile + ivec2(x,y));

            ivec2 pixel_pos = (ivec2(x,y) + base_tile) * tile_size - world.camera.Pos();

            switch (info.draw_mode)
            {
              case invis:
                // Nothing.
                break;
              case fancy:
                {
                    int mask = 0;
                    for (int i = 0; i < 8; i++)
                    {
                        // Offsets (8): (1,0), (1,1), (0,1), (-1,1), ..., (1,-1).
                        ivec2 offset(1 - (i >= 2) - (i >= 3) + (i >= 6) + (i >= 7), (i >= 1) - (i >= 4) - (i >= 5));
                        bool should_merge = ClampGet(base_tile + ivec2(x,y) + offset).*layer == index;
                        mask <<= 1;
                        mask |= should_merge;
                    }

                    ivec2 tile_state;

                         if ((mask & 0b10111110) == 0b00111110) tile_state = ivec2(1, r % 4);     // #]
                    else if ((mask & 0b10101111) == 0b10001111) tile_state = ivec2(2 + r % 4, 1); // __
                    else if ((mask & 0b11101011) == 0b11100011) tile_state = ivec2(0, r % 4);     // [#
                    else if ((mask & 0b11111010) == 0b11111000) tile_state = ivec2(2 + r % 4, 0); // ^^
                    else if ((mask & 0b10101110) == 0b00001110) tile_state = ivec2(3, 3);         // _]
                    else if ((mask & 0b10101011) == 0b10000011) tile_state = ivec2(2, 3);         // [_
                    else if ((mask & 0b11101010) == 0b11100000) tile_state = ivec2(2, 2);         // [^
                    else if ((mask & 0b10111010) == 0b00111000) tile_state = ivec2(3, 2);         // ^]
                    else if ((mask & 0b11111111) == 0b10111111) tile_state = ivec2(4, 2);         // ]_
                    else if ((mask & 0b11111111) == 0b11101111) tile_state = ivec2(5, 2);         // _[
                    else if ((mask & 0b11111111) == 0b11111011) tile_state = ivec2(5, 3);         // ^[
                    else if ((mask & 0b11111111) == 0b11111110) tile_state = ivec2(4, 3);         // ]^
                    else                                        tile_state = ivec2(6, r % 4);     // ##

                    render.iquad(pixel_pos, ivec2(tile_size)).tex(img_tiles.pos + tile_state.add_y(info.image_index * 4) * tile_size);
                }
                break;
              case cover:
                {
                    bool merge_left = ClampGet(base_tile + ivec2(x-1,y)).*layer == index;
                    bool merge_right = ClampGet(base_tile + ivec2(x+1,y)).*layer == index;
                    int state;
                         if (merge_left && !merge_right) state = 5;
                    else if (!merge_left && merge_right) state = 4;
                    else                                 state = r % 4;

                    render.iquad(pixel_pos, ivec2(tile_size)).tex(img_tiles.pos + ivec2(7 + state, info.image_index) * tile_size);
                }
                break;
            }
        }
    }
}
