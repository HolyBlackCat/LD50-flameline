#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <map>
#include <vector>

#include "reflection/full.h"
#include "utils/mat.h"
#include "utils/multiarray.h"


// Converts a tile map to multiple oriented edge lists, suitable for Box2D (or something else).

namespace GameUtils::TilesToEdges
{
    // Describes a tile set, i.e. shapes of specific tiles.
    struct TileSet
    {
        REFL_SIMPLE_STRUCT( Params
            REFL_DECL(ivec2 REFL_INIT{}) tile_size
            // Vertex positions.
            REFL_DECL(std::vector<ivec2>) vertices
            // The outer vector describes tile types.
            // The middle vector decribes edge loops in a tile.
            // The inner vector stores vertex indices of a specific edge loop.
            // Box2D requires the vertices to have a specific winding (CW if Y points down, CCW if Y points up).
            REFL_DECL(std::vector<std::vector<std::vector<std::size_t>>>) tiles
        )

        TileSet() {}

        // Creates a tile set from `params`.
        // The alternative is manually setting the variables and calling the functions below.
        TileSet(Params params);

        // If you generate the tileset manually (rather than from `Params`), call this to validate it.
        // Throws on failure.
        void Validate();

        ivec2 tile_size{};

        // Vertex positions, normally in range 0..tile_size.
        // Positions must be unique.
        std::vector<ivec2> vertices;

        // All vertex indices used by a tile, arranged in separate loops.
        std::vector<std::vector<std::vector<std::size_t>>> tile_vertices;

        // Maps a vertex and a 8-dir to a an overlapping vertex in a tile in that direction, or -1 if none.
        std::vector<std::array<std::size_t, 8>> matching_vertices;

        // Maps a tile index + a vertex index to an edge index, or `-1` if none.
        Array2D<std::size_t> edge_starting_at;

        // Maps edge indices to pairs of vertex indices.
        std::vector<std::pair<std::size_t, std::size_t>> edge_points;

        // Maps an edge and a 8-dir (4..7 only) to a symmetric edge that cancels out the original, or -1 if no such edge.
        std::vector<std::array<std::size_t, 4>> symmetric_edges;
    };

    using OutputCallback = std::function<void(ivec2 pos, bool last)>;

    struct Params
    {
        // The tile set, must be non-null.
        const TileSet *tileset = nullptr;

        // The map itself.
        Array2D<std::size_t> tiles;

        // The resulting edge vertices will be piped to this function.
        // Vertices form loops. `last == true` ends the loop, in that case `pos` will be the same as the first vertex in the loop.
        OutputCallback output_vertex;
    };

    // Performs the tiles->edges conversion.
    void Convert(const Params &params);
}
