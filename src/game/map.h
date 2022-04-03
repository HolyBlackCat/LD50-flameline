#pragma once

inline constexpr int tile_size = 12;

enum class Tile
{
    air,
    wall,
    spike_up,
    spike_down,
    spike_left,
    spike_right,
    box1,
    box2,
    chain_h,
    chain_v,
    breakable,
    _count,
};

struct TileInfo
{
    Tile tile;
    bool solid = false;
    bool kills = false;
    bool breakable = false;

    int simple_tex = -1; // If not -1, a texture index for simple drawing.

    bool is_dual_grid_tile = false; // If true, rendered using a dual grid.

    int spike_like_dir = -1;
    int spike_like_tex = 0; // Texture index, if `spike_like_dir != -1`.
    bool spike_like_merge_with_any_solid = false;
};

inline constexpr TileInfo tile_info[] = {
    { .tile = Tile::air,         },
    { .tile = Tile::wall,        .solid = true, .is_dual_grid_tile = true, },
    { .tile = Tile::spike_up,    .kills = true, .spike_like_dir = 0, .spike_like_tex = 0, },
    { .tile = Tile::spike_down,  .kills = true, .spike_like_dir = 2, .spike_like_tex = 0, },
    { .tile = Tile::spike_left,  .kills = true, .spike_like_dir = 3, .spike_like_tex = 0, },
    { .tile = Tile::spike_right, .kills = true, .spike_like_dir = 1, .spike_like_tex = 0, },
    { .tile = Tile::box1,        .solid = true, .simple_tex = 2, },
    { .tile = Tile::box2,        .solid = true, .spike_like_dir = 0, .spike_like_tex = 3, },
    { .tile = Tile::chain_h,                    .spike_like_dir = 0, .spike_like_tex = 5, .spike_like_merge_with_any_solid = true, },
    { .tile = Tile::chain_v,                    .spike_like_dir = 1, .spike_like_tex = 5, .spike_like_merge_with_any_solid = true, },
    { .tile = Tile::breakable,   .solid = true, .breakable = true, .simple_tex = 7, },
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
    Array2D<unsigned char> random;

    ivec2 player_start;
    std::optional<ivec2> debug_player_start;
    float initial_lava_level = 0;
    float exit_level = 0;

    std::optional<ivec2> ability_timeshift;
    bool debug_start_with_timeshift = false;

    std::optional<ivec2> ability_doublejump;
    bool debug_start_with_doublejump = false;

    std::optional<ivec2> ability_gun;
    bool debug_start_with_gun = false;

    std::vector<ivec2> secrets;
    int num_secrets = 0;

    Tiled::PointLayer points;

    MAYBE_CONST(
        [[nodiscard]] CV Cell &at(ivec2 pos) CV
        {
            return cells.clamped_at(pos);
        }

        [[nodiscard]] CV Cell &at_pixel(ivec2 pixel_pos) CV
        {
            return at(div_ex(pixel_pos, tile_size));
        }
    )

    [[nodiscard]] unsigned char rand_at(ivec2 pos) const
    {
        return random.unsafe_at(mod_ex(pos, random.size()));
    }

    [[nodiscard]] ivec2 size() const
    {
        return cells.size();
    }

    Map(Stream::ReadOnlyData data);

    void render(ivec2 camera_pos) const;
};
