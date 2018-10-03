#include "program/parachute.h"

#include <utility>
#include <vector>

#include "game/states/world/tile_map.h"
#include "game/meta.h"

namespace States::Details::World
{
    static const std::vector<TileMap::TileInfo> tile_list
    {
        {"moss_stone"}, // 0
    };


    TileMap::TileImages::TileImages(const std::string &name)
    {
        auto refl = Refl::Interface(*this);
        refl.for_each_field([&](auto index)
        {
            constexpr int i = index.value;
            refl.field_value<i>() = Atlas().GetList(name + "." + refl.field_name(i) + ".", 0, ".png");
        });
    }

    const TileMap::TileInfo &TileMap::GetTileInfo(int index)
    {
        if (index < 0 || index >= int(tile_list.size()))
            Program::Error("Tile index ", index, " is out of range.");
        return tile_list[index];
    }

    void TileMap::Load(const std::string &name)
    {
        (void)name;
    }

    void TileMap::Render() const
    {
        render.iquad(-screen_size/2, screen_size).color(fvec3(0), fvec3(0), fvec3(0.1), fvec3(0.1));
        //render.iquad(mouse.pos(), tile_list[0].img[0][0].size).tex(tile_list[0].img[0][0].pos).center().rotate(sin(metronome.ticks / 100.));
    }
}
