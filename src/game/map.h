#pragma once

inline constexpr int tile_size = 12;

enum class Tile
{
    air,
    wall,
    _count,
};

struct TileInfo
{
    Tile tile;
    bool solid = false;

    bool uses_dual_grid = false;
};

inline constexpr TileInfo tile_info[] = {
    { .tile = Tile::air,  .solid = false, .uses_dual_grid = false, },
    { .tile = Tile::wall, .solid = true,  .uses_dual_grid = true,  },
};

static_assert(std::size(tile_info) == std::size_t(Tile::_count), "In tile info array: incorrect array size.");
static_assert([]{
    for (std::size_t i = 0; i < std::size(tile_info); i++)
    {
        if (tile_info[i].tile != Tile(i))
            return false;
    }
    return true;
}, "In tile info array: incorrect tile enum.");

struct Cell
{
    Tile tile{};

    [[nodiscard]] const TileInfo &info() const
    {
        if (tile < Tile{} || tile >= Tile::_count)
            throw std::runtime_error("Invalid tile enum.");
        return tile_info[std::size_t(tile)];
    }
};

struct Map
{
    Array2D<Cell> cells;

    MAYBE_CONST(
        [[nodiscard]] CV Cell &at(ivec2 pos) CV
        {
            return cells.clamped_at(pos);
        }
    )

    [[nodiscard]] ivec2 size() const
    {
        return cells.size();
    }

    Map(Stream::ReadOnlyData data);

    void render(ivec2 camera_pos) const;
};
