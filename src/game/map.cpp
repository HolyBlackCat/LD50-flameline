#include "map.h"

#include "main.h"

Map::Map(Stream::ReadOnlyData data)
{
    Json json(data.string(), 32);
    auto tiles = Tiled::LoadTileLayer(Tiled::FindLayer(json.GetView(), "mid"));

    cells = Array2D<Cell>(tiles.size());
    random = Array2D<unsigned char>(tiles.size());

    for (auto pos : vector_range(cells.size()))
    {
        auto tile = Tile(tiles.unsafe_at(pos));
        if (tile < Tile{} || tile >= Tile::_count)
            throw std::runtime_error(FMT("Invalid tile index {} at {}.", int(tile), pos));

        Cell &cell = cells.unsafe_at(pos);
        cell.tile = tile;

        random.unsafe_at(pos) = ra.i <= 255;
    }

    points = Tiled::LoadPointLayer(Tiled::FindLayer(json.GetView(), "points"));

    player_start = points.GetSinglePoint("player");
    debug_player_start = points.GetSinglePointOpt("debug_player");
    initial_lava_level = points.GetSinglePoint("lava").y;
    ability_timeshift = points.GetSinglePointOpt("ability_timeshift");
    debug_start_with_timeshift = points.GetSinglePointOpt("debug_give_timeshift").has_value();
}

void Map::render(ivec2 camera_pos) const
{
    static const auto &region = texture_atlas.Get("tiles.png");

    ivec2 corner_a = div_ex(camera_pos - screen_size / 2, tile_size);
    ivec2 corner_b = div_ex(camera_pos + screen_size / 2, tile_size);

    { // Bottom layer.
        for (ivec2 tile_pos : corner_a <= vector_range <= corner_b)
        {
            const Cell &cell = at(tile_pos);
            const TileInfo &info = cell.info();
            if (info.spike_like_dir != -1)
            {
                int sign = info.spike_like_dir == 1 ? -1 : 1;

                ivec2 dir = ivec2::dir4(info.spike_like_dir);
                fmat2 mat(dir, dir.rot90());
                ivec2 offset_a = ivec2(-1 * sign, 0).rot90(info.spike_like_dir);
                ivec2 offset_b = ivec2( 1 * sign, 0).rot90(info.spike_like_dir);
                const Cell &cell_a = at(tile_pos + offset_a);
                const Cell &cell_b = at(tile_pos + offset_b);
                bool same_a = cell_a.tile == cell.tile || (info.spike_like_merge_with_any_solid && cell_a.info().solid);
                bool same_b = cell_b.tile == cell.tile || (info.spike_like_merge_with_any_solid && cell_b.info().solid);

                ivec2 pixel_pos = tile_pos * tile_size + tile_size/2 - camera_pos;
                r.iquad(pixel_pos, region.region(ivec2(0, tile_size * (info.spike_like_tex + same_a)), ivec2(tile_size) with(x /= 2))).center(ivec2(tile_size/2)).matrix(mat).flip_x(sign < 0);
                r.iquad(pixel_pos, region.region(ivec2(tile_size/2, tile_size * (info.spike_like_tex + same_b)), ivec2(tile_size) with(x /= 2))).center(ivec2(0, tile_size/2)).matrix(mat).flip_x(sign < 0);
            }
        }
    }

    { // Dual-grid layer.
        ivec2 dual_corner_a = div_ex(camera_pos - screen_size / 2 - tile_size / 2, tile_size);
        ivec2 dual_corner_b = div_ex(camera_pos + screen_size / 2 - tile_size / 2, tile_size);


        for (ivec2 tile_pos : dual_corner_a <= vector_range <= dual_corner_b)
        {
            { // Dual grid.
                int bits = 0;
                for (ivec2 tile_offset : vector_range(ivec2(2)))
                {
                    bits |= 16 * at(tile_pos + tile_offset).info().is_dual_grid_tile;
                    bits >>= 1;
                }

                ivec2 variant(bits % 4, bits / 4);
                // if (bits == 15)
                // {
                //     int randvar = rand_at(tile_pos) / 8;
                //     if (randvar < 4)
                //         variant = ivec2(4, randvar);
                // }

                ivec2 dual_pixel_pos = tile_pos * tile_size + tile_size / 2 - camera_pos;
                r.iquad(dual_pixel_pos, region.region((variant + ivec2(1, 0)) * tile_size, ivec2(tile_size)));
            }
        }
    }

    { // Top layer.
        for (ivec2 tile_pos : corner_a <= vector_range <= corner_b)
        {
            const Cell &cell = at(tile_pos);
            const TileInfo &info = cell.info();
            if (info.simple_tex != -1)
            {
                r.iquad(tile_pos * tile_size - camera_pos, region.region(ivec2(0, tile_size * info.simple_tex), ivec2(tile_size)));
            }
        }
    }
}
