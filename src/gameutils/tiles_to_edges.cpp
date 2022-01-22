#include "tiles_to_edges.h"

#include "strings/format.h"
#include "utils/bit_vectors.h"
#include "utils/multiarray.h"

namespace GameUtils::TilesToEdges
{
    TileSet::TileSet(Params params)
        : tile_size(params.tile_size), vertices(std::move(params.vertices)), tile_vertices(std::move(params.tiles))
    {
        // Generate vertex to id mapping.
        std::map<ivec2, std::size_t> vertex_ids;

        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            bool ok = vertex_ids.try_emplace(vertices[i], i).second;
            if (!ok)
                throw std::runtime_error(FMT("Duplicate position {} for vertex {}.", vertices[i], i));
        }

        // Find overlapping vertices in adjacent tiles.
        matching_vertices.resize(vertices.size());
        for (std::size_t vertex = 0; vertex < vertices.size(); vertex++)
        {
            for (int dir = 0; dir < 8; dir++)
            {
                auto it = vertex_ids.find(vertices[vertex] - ivec2::dir8(dir) * tile_size);
                matching_vertices[vertex][dir] = it == vertex_ids.end() ? -1zu : it->second;
            }
        }

        // Generate edge ids.
        std::map<std::pair<std::size_t, std::size_t>, std::size_t> edge_ids;

        for (const auto &tile_loops : tile_vertices)
        for (const auto &tile_loop : tile_loops)
        {
            if (tile_loop.empty())
                throw std::runtime_error("Empty vertex loops are not allowed.");
            std::size_t prev_vertex = tile_loop.back();
            for (std::size_t vertex : tile_loop)
            {
                if (vertex >= vertices.size())
                    throw std::runtime_error(FMT("Vertex index {} is out of range.", vertex));
                edge_ids.try_emplace(std::pair(prev_vertex, vertex), edge_ids.size());
                prev_vertex = vertex;
            }
        }

        // Fill [tile,vertex] to edge mapping.
        edge_starting_at = Array2D<std::size_t>(index_vec2(tile_vertices.size(), vertices.size()), -1zu);

        for (std::size_t tile = 0; tile < tile_vertices.size(); tile++)
        {
            for (const auto &tile_loop : tile_vertices[tile])
            {
                // We already checked for empty loops above.
                std::size_t prev_vertex = tile_loop.back();
                for (std::size_t vertex : tile_loop)
                {
                    std::size_t &cell = edge_starting_at.safe_nonthrowing_at(index_vec2(tile, prev_vertex));
                    if (cell != -1zu)
                        throw std::runtime_error(FMT("In tile type {}: Multiple edges begin at vertex {}.", tile, prev_vertex));
                    cell = edge_ids.at(std::pair(prev_vertex, vertex));
                    prev_vertex = vertex;
                }
            }
        }

        // Fill edge to end vertex mapping.
        edge_points.resize(edge_ids.size());
        for (const auto &[vertex_pair, edge] : edge_ids)
            edge_points[edge] = vertex_pair;

        // Fill symmetric edge info.
        symmetric_edges.resize(edge_ids.size());
        for (int i = 0; i < 4; i++)
        {
            ivec2 offset = ivec2::dir8(i) * tile_size;

            for (const auto &[vertex_pair, edge] : edge_ids)
            {
                symmetric_edges[edge][i] = -1zu;
                auto vert_it_a = vertex_ids.find(vertices[vertex_pair.second] + offset);
                if (vert_it_a == vertex_ids.end())
                    continue;
                auto vert_it_b = vertex_ids.find(vertices[vertex_pair.first] + offset);
                if (vert_it_b == vertex_ids.end())
                    continue;
                auto edge_it = edge_ids.find(std::pair(vert_it_a->second, vert_it_b->second));
                if (edge_it == edge_ids.end())
                    continue;
                symmetric_edges[edge][i] = edge_it->second;
            }
        }

        // Validate, just in case.
        Validate();
    }

    void TileSet::Validate()
    {
        // Check tile size.
        if ((tile_size <= 0).any())
            throw std::runtime_error("Tile size must be positive.");

        // Check vertex count.
        if (vertices.size() != std::size_t(edge_starting_at.size().y) || vertices.size() != matching_vertices.size())
            throw std::runtime_error("Vertex count mismatch between `vertices`, `edge_starting_at`, and `matching_vertices`.");

        // Check edge count.
        if (edge_points.size() != symmetric_edges.size())
            throw std::runtime_error("Edge count mismatch between `edge_points` and `symmetric_edges`.");

        // Check tile count.
        if (tile_vertices.size() != std::size_t(edge_starting_at.size().x))
            throw std::runtime_error("Tile count mismatch between `tile_vertices` and `edge_starting_at`.");

        for (const auto &tile_loops : tile_vertices)
        for (const auto &tile_loop : tile_loops)
        {
            if (tile_loop.empty())
                throw std::runtime_error("Empty vertex loops are not allowed.");

            for (std::size_t vertex : tile_loop)
            {
                if (vertex >= vertices.size())
                    throw std::runtime_error("Invalid vertex in `tile_vertices`.");
            }
        }

        for (const auto &vert_list : matching_vertices)
        for (std::size_t vertex : vert_list)
        {
            if (vertex != -1zu && vertex >= vertices.size())
                throw std::runtime_error("Invalid vertex in `matching_vertices`.");
        }

        for (index_vec2 pos : vector_range(edge_starting_at.size()))
        {
            std::size_t edge = edge_starting_at.safe_throwing_at(pos);
            if (edge != -1zu && edge >= edge_points.size())
                throw std::runtime_error("Invalid edge in `edge_starting_at`.");
        }

        for (auto [a, b] : edge_points)
        {
            if (a >= vertices.size() || b >= vertices.size())
                throw std::runtime_error("Invalid vertex in `edge_points`.");
        }

        for (const auto &edges : symmetric_edges)
        for (std::size_t edge : edges)
        {
            if (edge != -1zu && edge >= edge_points.size())
                throw std::runtime_error("Invalid edge in `symmetric_edges`.");
        }
    }

    void Convert(const Params &params)
    {
        const TileSet &tileset = *params.tileset;

        // A type for bit masks.
        using bits_t = std::uint64_t;


        // Which edges in each tile were already processed.
        // All sub-arrays have the same size, matching the map size.
        // The outer vector is used in case the mask doesn't fit into a single `bits_t`.
        std::vector<Array2D<bits_t>> valid_edges((tileset.edge_points.size() + BitVec::bit_width<bits_t> - 1) / BitVec::bit_width<bits_t>);
        for (Array2D<bits_t> &arr : valid_edges)
            arr = Array2D<bits_t>(params.tiles.size());

        // Fill the edge bit array, and remove the conflicting edges.
        for (index_vec2 tile_pos : vector_range(params.tiles.size()))
        {
            std::size_t tile = params.tiles.unsafe_at(tile_pos);
            if (tile >= tileset.tile_vertices.size())
                throw std::runtime_error(FMT("Tile index {} at {} is out of range.", tile, tile_pos));

            for (const auto &vertex_loop : tileset.tile_vertices[tile])
            for (std::size_t vertex : vertex_loop)
            {
                std::size_t edge = tileset.edge_starting_at.safe_nonthrowing_at(index_vec2(tile, vertex));

                bool ok = true;
                for (int dir = 0; dir < 4; dir++)
                {
                    ivec2 other_tile_pos = tile_pos + ivec2::dir8(dir + 4);

                    // Note that we don't check the upper limit of Y here, because the offset can never have a positive Y.
                    if ((other_tile_pos < 0).any() || other_tile_pos.x >= params.tiles.size().x)
                        continue; // No tile in that direction.

                    std::size_t other_edge = tileset.symmetric_edges[edge][dir];
                    if (other_edge == -1zu)
                        continue;

                    auto loc = BitVec::BitLocation<bits_t>(other_edge);
                    bits_t &bits = valid_edges[loc.index].safe_nonthrowing_at(other_tile_pos);
                    if (bits & loc.mask)
                    {
                        // Found a conflicting edge, remove it and stop.
                        ok = false;
                        bits &= ~loc.mask;
                        break;
                    }
                }

                // If no conflicting edges, create this edge.
                if (ok)
                {
                    auto loc = BitVec::BitLocation<bits_t>(edge);
                    valid_edges[loc.index].safe_nonthrowing_at(tile_pos) |= loc.mask;
                }
            }
        }

        // Generate loops from the edges.
        for (const index_vec2 starting_tile_pos : vector_range(params.tiles.size()))
        {
            const std::size_t starting_tile = params.tiles.unsafe_at(starting_tile_pos);

            for (const auto &vertex_loop : tileset.tile_vertices[starting_tile])
            for (const std::size_t pre_starting_vertex : vertex_loop)
            {
                const std::size_t starting_edge = tileset.edge_starting_at.safe_nonthrowing_at(index_vec2(starting_tile, pre_starting_vertex));
                if (auto loc = BitVec::BitLocation<bits_t>(starting_edge); !(valid_edges[loc.index].safe_nonthrowing_at(starting_tile_pos) & loc.mask))
                    continue; // This edge doesn't exist.

                const std::size_t starting_vertex = tileset.edge_points[starting_edge].second;

                ivec2 tile_pos = starting_tile_pos;
                std::size_t vertex = starting_vertex;
                std::size_t edge = starting_edge;
                ivec2 dir = tileset.vertices[starting_vertex] - tileset.vertices[pre_starting_vertex];

                bool first = true;

                while (true)
                {
                    bool finished = !first && tile_pos == starting_tile_pos && vertex == starting_vertex;

                    params.output_vertex(tile_pos * params.tileset->tile_size + tileset.vertices[vertex], finished);

                    // Remove the visited edge.
                    if (!first)
                    {
                        auto loc = BitVec::BitLocation<bits_t>(edge);
                        valid_edges[loc.index].safe_nonthrowing_at(tile_pos) &= ~loc.mask;
                    }

                    first = false;

                    if ( finished )
                        break;

                    bool found_any = false;
                    ivec2 best_tile_pos(0);
                    std::size_t best_vertex = -1zu;
                    std::size_t best_edge = -1zu;
                    ivec2 best_dir(0);

                    for (int i = -1; i < 8; i++)
                    {
                        std::size_t next_vertex_a = i == -1 ? vertex : tileset.matching_vertices[vertex][i];
                        if (i != -1 && next_vertex_a == -1zu)
                            continue; // No matching vertex.
                        ivec2 next_tile_pos = tile_pos + (i == -1 ? ivec2() : ivec2::dir8(i));
                        if (i != -1 && ((next_tile_pos < 0).any() || (next_tile_pos >= params.tiles.size()).any()))
                            continue; // Out of bounds.

                        std::size_t next_tile = params.tiles.safe_nonthrowing_at(next_tile_pos);
                        std::size_t next_edge = tileset.edge_starting_at.safe_nonthrowing_at(index_vec2(next_tile, next_vertex_a));
                        if (next_edge == -1zu)
                            continue; // No edge.

                        if (auto loc = BitVec::BitLocation<bits_t>(next_edge); !(valid_edges[loc.index].safe_nonthrowing_at(next_tile_pos) & loc.mask))
                            continue; // Already visited that edge.

                        std::size_t next_vertex_b = params.tileset->edge_points[next_edge].second;

                        ivec2 next_dir = tileset.vertices[next_vertex_b] - tileset.vertices[next_vertex_a];

                        if (!found_any || Math::less_positively_rotated(-dir, next_dir, best_dir))
                        {
                            found_any = true;
                            best_tile_pos = next_tile_pos;
                            best_vertex = next_vertex_b;
                            best_edge = next_edge;
                            best_dir = next_dir;
                        }
                    }

                    if (!found_any)
                        throw std::runtime_error(FMT("Edge loop unexpectedly ended at tile {} vertex {}.", tile_pos, vertex));

                    tile_pos = best_tile_pos;
                    vertex = best_vertex;
                    edge = best_edge;
                    dir = best_dir;
                }
            }
        }
    }
}

/* Some test cases:

std::string tileset_str = R"(
{
    tile_size = (16,16),
    vertices = [
        (0,0),
        (16,0),
        (16,16),
        (0,16),
    ],
    tiles = [
        [],
        [[0,1,2,3,]], // 1 []
        [[0,1,  3,]], // 2 |/
        [[0,1,2,  ]], // 3 \|
        [[  1,2,3,]], // 4 /|
        [[0,  2,3,]], // 5 |\
    ],
}
)";

Array2D<std::size_t> map(Meta::value_list<4,4>{},
    {
        0,0,0,0,
        0,1,0,0,
        0,0,0,0,
        0,0,0,0,
    }
);


Array2D<std::size_t> map(Meta::value_list<4,4>{},
    {
        0,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,0,
    }
);

Array2D<std::size_t> map(Meta::value_list<4,4>{},
    {
        0,0,0,0,
        0,3,4,0,
        0,2,5,0,
        0,0,0,0,
    }
);

Array2D<std::size_t> map(Meta::value_list<4,4>{},
    {
        0,0,0,0,
        0,1,1,0,
        0,0,1,0,
        0,0,0,0,
    }
);

Array2D<std::size_t> map(Meta::value_list<4,5>{},
    {
        0,0,0,0,
        0,1,1,1,
        0,1,0,1,
        0,1,1,1,
        0,0,0,0,
    }
);

Array2D<std::size_t> map(Meta::value_list<20,6>{},
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,0,1,0,1,1,1,0,1,0,0,0,1,0,0,0,1,1,1,
        0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,
        0,1,1,1,0,1,1,0,0,1,0,0,0,1,0,0,0,1,0,1,
        0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,
        0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,
    }
);

auto tileset_params = Refl::FromString<GameUtils::TilesToEdges::TileSet::Params>(tileset_str);
GameUtils::TilesToEdges::TileSet tileset(tileset_params);
GameUtils::TilesToEdges::Params params;
params.tileset = &tileset;
params.tiles = map;
params.output_vertex = [&](ivec2 pos, bool last){std::cout << pos << last << '\n';};

GameUtils::TilesToEdges::Convert(params);

*/
