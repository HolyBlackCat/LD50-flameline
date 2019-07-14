#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <vector>

#include "graphics/index_buffer.h"
#include "graphics/vertex_buffer.h"
#include "program/errors.h"
#include "utils/meta.h"

namespace Graphics
{
    template <typename V, typename I = std::uint16_t>
    class IndexedGeometryConsumer : Meta::with_virtual_destructor<IndexedGeometryConsumer<V, I>>
    {
        static_assert(std::is_same_v<I, std::uint8_t> || std::is_same_v<I, std::uint16_t> || std::is_same_v<I, std::uint32_t>, "Invalid index type.");

      public:
        using vertex_t = V;
        using index_t = I;

        virtual void InsertVertices(const V *vertex_ptr, std::size_t vertex_count) = 0;
        virtual void InsertVerticesAndIndices(const V *vertex_ptr, std::size_t vertex_count, const I *index_ptr, std::size_t index_count) = 0;

        virtual void Flush() {}
    };

    template <typename V, typename I = std::uint16_t>
    class IndexedVertexData : public IndexedGeometryConsumer<V, I>
    {
      public:
        std::vector<V> vertices;
        std::vector<I> indices;

        void InsertVertices(const V *vertex_ptr, std::size_t vertex_count) override
        {
            DebugAssert("Too many vertices for this index type.", vertices.size() + vertex_count - 1 <= std::numeric_limits<I>::max());
            I base_index = vertices.size();
            vertices.insert(vertices.end(), vertex_ptr, vertex_ptr + vertex_count);
            indices.reserve(indices.size() + vertex_count);
            for (std::size_t i = 0; i < vertex_count; i++)
                indices.push_back(base_index + i);
        }

        void InsertVerticesAndIndices(const V *vertex_ptr, std::size_t vertex_count, const I *index_ptr, std::size_t index_count) override
        {
            I base_index = vertices.size();
            vertices.insert(vertices.end(), vertex_ptr, vertex_ptr + vertex_count);
            indices.reserve(indices.size() + index_count);
            for (std::size_t i = 0; i < index_count; i++)
            {
                I this_index = index_ptr[i];
                DebugAssert("Vertex index is out of range.", this_index < vertex_count);
                indices.push_back(base_index + this_index);
            }
        }
    };

    template <typename V, typename I = std::uint16_t>
    class IndexedGeometry
    {
        DrawMode mode = triangles;
        VertexBuffer<V> vertex_buffer;
        IndexBuffer<I> index_buffer;

      public:
        IndexedGeometry() {}
        IndexedGeometry(const IndexedVertexData<V, I> &vertex_data, DrawMode mode, Usage usage = static_draw)
            : mode(mode), vertex_buffer(vertex_data.vertices.size(), vertex_data.vertices.data(), usage), index_buffer(vertex_data.indices.size(), vertex_data.indices.data(), usage)
        {}

        explicit operator bool() const
        {
            return bool(vertex_buffer);
        }

        void Draw()
        {
            vertex_buffer.BindDraw();
            index_buffer.Draw(mode);
        }
    };
}
