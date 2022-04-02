#include "map.h"

#include "main.h"

Map::Map(Stream::ReadOnlyData data)
{
    Json json(data.string(), 32);
    auto tiles = Tiled::LoadTileLayer(Tiled::FindLayer(json.GetView(), "mid"));

    cells = Array2D<Cell>(tiles.size());

    for (auto pos : vector_range(cells.size()))
    {
        auto tile = Tile(tiles.unsafe_at(pos));
        if (tile < Tile{} || tile >= Tile::_count)
            throw std::runtime_error(FMT("Invalid tile index {} at {}.", int(tile), pos));
        cells.unsafe_at(pos).tile = tile;
    }
}

void Map::render(ivec2 camera_pos) const
{
    ivec2 corner_a = div_ex(camera_pos - screen_size / 2 - tile_size / 2, tile_size);
    ivec2 corner_b = div_ex(camera_pos + screen_size / 2 - tile_size / 2, tile_size);

    static const auto &region = texture_atlas.Get("tiles.png");

    for (ivec2 tile_pos : corner_a <= vector_range <= corner_b)
    {
        { // Dual grid.
            int bits = 0;
            for (ivec2 tile_offset : vector_range(ivec2(2)))
            {
                bits |= 16 * at(tile_pos + tile_offset).info().uses_dual_grid;
                bits >>= 1;
            }

            ivec2 variant(bits % 4, bits / 4);

            ivec2 dual_pixel_pos = tile_pos * tile_size + tile_size / 2 - camera_pos;
            r.iquad(dual_pixel_pos, region.region(variant * tile_size, ivec2(tile_size)));
        }
    }
}
