#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include "graphics/index_buffer.h"
#include "graphics/vertex_buffer.h"
#include "program/errors.h"
#include "utils/check.h"
#include "utils/meta.h"

namespace Graphics::Geom
{
    // Generic interface for vertex containers

    template <typename V>
    class ProviderIndexless : Meta::with_virtual_destructor<ProviderIndexless<V>>
    {
      public:
        using vertex_t = V;

      protected:
        virtual void GetVerticesFlatLow(std::size_t begin, std::size_t end, vertex_t *dest) const = 0;

      public:
        virtual std::size_t VertexCountFlat() const = 0;

        void GetVerticesFlat(std::size_t begin, std::size_t end, vertex_t *dest) const
        {
            DebugAssert("Invalid vertex range.", begin <= end && end <= VertexCountFlat());
            GetVerticesFlatLow(begin, end, dest);
        }
    };

    template <typename V, typename I>
    class Provider : public ProviderIndexless<V>
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");

      public:
        using typename ProviderIndexless<V>::vertex_t;
        using index_t = I;

      protected:
        virtual void GetVerticesLow(std::size_t begin, std::size_t end, vertex_t *dest) const = 0;
        virtual void GetIndicesLow(std::size_t begin, std::size_t end, index_t *dest, index_t base_index) const = 0;

      public:
        virtual std::size_t VertexCount() const = 0;
        virtual std::size_t IndexCount() const = 0;

        void GetVertices(std::size_t begin, std::size_t end, vertex_t *dest) const
        {
            DebugAssert("Invalid vertex range.", begin <= end && end <= VertexCount());
            GetVerticesLow(begin, end, dest);
        }
        void GetIndices(std::size_t begin, std::size_t end, index_t *dest, index_t base_index) const
        {
            DebugAssert("Invalid index range.", begin <= end && end <= IndexCount());
            GetIndicesLow(begin, end, dest, base_index);
        }
    };


    // Generic interface for vertex containers with contiguous storage

    template <typename V>
    class ArrayProviderIndexless : public ProviderIndexless<V>
    {
      public:
        using typename ProviderIndexless<V>::vertex_t;

        const vertex_t *VertexPointerFlat() const = 0;
    };

    template <typename V, typename I>
    class ArrayProvider : public Provider<V, I>
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");

      public:
        using typename Provider<V, I>::vertex_t;
        using typename Provider<V, I>::index_t;

        const vertex_t *VertexPointer() const = 0;
        const index_t *IndexPointer() const = 0;
    };


    // Implementations of `ArrayProvider[Indexless]` pointing to existing storage

    template <typename V>
    class ViewIndexless : public ArrayProviderIndexless<V>
    {
        using Base = ProviderIndexless<V>;

      public:
        using typename Base::vertex_t;

      private:
        const vertex_t *vertex_ptr = nullptr;
        std::size_t vertex_count = 0;

      protected:
        void GetVerticesFlatLow(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            std::copy(vertex_ptr + begin, vertex_ptr + end, dest);
        }

      public:
        ViewIndexless() {}
        ViewIndexless(const vertex_t *vertex_ptr, std::size_t vertex_count)
            : vertex_ptr(vertex_ptr), vertex_count(vertex_count)
        {}

        std::size_t VertexCountFlat() const override
        {
            return vertex_count;
        }

        const vertex_t *VertexPointerFlat() const override
        {
            return vertex_ptr;
        }
    };

    template <typename V, typename I>
    class ViewNonIndexBased : public Provider<V, I>
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");
        using Base = Provider<V, I>;

      public:
        using typename Base::vertex_t;
        using typename Base::index_t;

      private:
        ViewIndexless<V> source;

      protected:
        void GetVerticesFlatLow(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            source.GetVerticesFlat(begin, end, dest);
        }
        void GetVerticesLow(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            source.GetVerticesFlat(begin, end, dest);
        }
        void GetIndicesLow(std::size_t begin, std::size_t end, index_t *dest, index_t base_index) const override
        {
            DebugAssert("Vertex index is too large for this type.", (end - begin - 1) + base_index <= std::numeric_limits<index_t>::max());
            std::iota(dest, dest + (end - begin), index_t(begin + base_index));
        }

      public:
        ViewNonIndexBased() {}
        ViewNonIndexBased(const vertex_t *vertex_ptr, std::size_t vertex_count) : source(vertex_ptr, vertex_count) {}
        ViewNonIndexBased(const ViewIndexless<V> &source) : source(source) {}

        std::size_t VertexCountFlat() const override
        {
            return source.VertexCountFlat();
        }
        std::size_t VertexCount() const override
        {
            return source.VertexCountFlat();
        }
        std::size_t IndexCount() const override
        {
            return source.VertexCountFlat();
        }
    };

    template <typename V, typename I>
    class View : public ArrayProvider<V, I>
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");
        using Base = Provider<V, I>;

      public:
        using typename Base::vertex_t;
        using typename Base::index_t;

      private:
        const vertex_t *vertex_ptr = nullptr;
        std::size_t vertex_count = 0;
        const index_t *index_ptr = nullptr;
        std::size_t index_count = 0;

      protected:
        void GetVerticesFlatLow(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            for (std::size_t i = begin; i < end; i++)
            {
                std::size_t index = index_ptr[i];
                DebugAssert("Invalid vertex index.", index < vertex_count);
                *dest++ = vertex_ptr[index];
            }
        }
        void GetVerticesLow(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            std::copy(vertex_ptr + begin, vertex_ptr + end, dest);
        }
        void GetIndicesLow(std::size_t begin, std::size_t end, index_t *dest, index_t base_index) const override
        {
            std::transform(index_ptr + begin, index_ptr + end, dest, [=](index_t index){return index + base_index;});
        }

      public:
        View() {}
        View(const vertex_t *vertex_ptr, std::size_t vertex_count, const index_t *index_ptr, std::size_t index_count)
            : vertex_ptr(vertex_ptr), vertex_count(vertex_count), index_ptr(index_ptr), index_count(index_count)
        {}

        std::size_t VertexCountFlat() const override
        {
            return index_count;
        }
        std::size_t VertexCount() const override
        {
            return vertex_count;
        }
        std::size_t IndexCount() const override
        {
            return index_count;
        }

        const vertex_t *VertexPointer() const override
        {
            return vertex_ptr;
        }
        const index_t *IndexPointer() const override
        {
            return index_ptr;
        }
    };


    // Vectors of vertices convertible to `ArrayProvider[Indexless]`

    template <typename V>
    class DataIndexless
    {
      public:
        using vertex_t = V;

        std::vector<vertex_t> vertices;

        DataIndexless() {}
        DataIndexless(std::vector<vertex_t> vertices) : vertices(std::move(vertices)) {}

        void Insert(const ProviderIndexless<V> &provider)
        {
            if (std::size_t added_vertex_count = provider.VertexCountFlat())
            {
                std::size_t old_vertex_count = vertices.size();
                vertices.resize(vertices.size() + added_vertex_count);
                provider.GetVerticesFlat(0, added_vertex_count, &vertices[old_vertex_count]);
            }
        }

        operator ViewIndexless<V>() const
        {
            return ViewIndexless<V>(vertices.data(), vertices.size());
        }
    };

    template <typename V, typename I>
    class Data
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");

      public:
        using vertex_t = V;
        using index_t = I;

        std::vector<vertex_t> vertices;
        std::vector<index_t> indices;

        Data() {}

        Data(std::vector<vertex_t> vertices, std::vector<index_t> indices) : vertices(std::move(vertices)), indices(std::move(indices))
        {
            DebugAssert("Some indices provided for a geometry are out of range.", std::all_of(indices.begin(), indices.end(), [&](index_t index){return index < vertices.size();}));
        }

        void Insert(const Provider<V, I> &provider)
        {
            std::size_t old_vertex_count = vertices.size();

            if (std::size_t added_vertex_count = provider.VertexCount())
            {
                vertices.resize(vertices.size() + added_vertex_count);
                provider.GetVertices(0, added_vertex_count, &vertices[old_vertex_count]);
            }

            if (std::size_t added_index_count = provider.IndexCount())
            {
                std::size_t old_index_count = indices.size();
                indices.resize(indices.size() + added_index_count);
                provider.GetIndices(0, added_index_count, &indices[old_index_count], old_vertex_count);
            }
        }

        operator View<V, I>() const
        {
            return View<V, I>(vertices.data(), vertices.size(), indices.data(), indices.size());
        }
    };


    // Fixed-size arrays of vertices convertible to `ArrayProvider[Indexless]`

    template <typename V, std::size_t VN>
    class DataFixedSizeIndexless
    {
      public:
        using vertex_t = V;

        std::array<vertex_t, VN> vertices;

        DataFixedSizeIndexless() {}
        DataFixedSizeIndexless(std::array<vertex_t, VN> vertices) : vertices(std::move(vertices)) {}

        operator ViewIndexless<V>() const
        {
            return ViewIndexless<V>(vertices.data(), vertices.size());
        }
    };

    template <typename V, std::size_t VN, typename I, std::size_t IN>
    class DataFixedSize
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");

      public:
        using vertex_t = V;
        using index_t = I;

        std::array<vertex_t, VN> vertices;
        std::array<index_t, IN> indices;

        DataFixedSize() {}
        DataFixedSize(std::array<vertex_t, VN> vertices, std::array<index_t, IN> indices)
            : vertices(std::move(vertices)), indices(std::move(indices)) {}

        operator View<V, I>() const
        {
            return View<V, I>(vertices.data(), vertices.size(), indices.data(), indices.size());
        }
    };


    // Drawable buffers that can be constructed from `ArrayProvider[Indexless]`

    template <typename V>
    class BufferIndexless
    {
        DrawMode mode = triangles;
        VertexBuffer<V> vertex_buffer;

      public:
        using vertex_t = V;

        BufferIndexless() {}
        BufferIndexless(const ArrayProviderIndexless<V> &data, DrawMode mode, Usage usage = static_draw)
            : mode(mode), vertex_buffer(data.VertexCountFlat(), data.VertexPointerFlat(), usage)
        {}

        explicit operator bool() const
        {
            return bool(vertex_buffer);
        }

        void Draw() const
        {
            vertex_buffer.Draw(mode);
        }
    };

    template <typename V, typename I>
    class Buffer
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");

        DrawMode mode = triangles;
        VertexBuffer<V> vertex_buffer;
        IndexBuffer<I> index_buffer;

      public:
        using vertex_t = V;
        using index_t = I;

        Buffer() {}
        Buffer(const ArrayProvider<V, I> &data, DrawMode mode, Usage usage = static_draw)
            : mode(mode), vertex_buffer(data.VertexCount(), data.VertexPointer(), usage), index_buffer(data.IndexCount(), data.IndexPointer(), usage)
        {}

        explicit operator bool() const
        {
            return bool(vertex_buffer);
        }

        void Draw() const
        {
            index_buffer.Draw(vertex_buffer, mode);
        }
    };


    // Rendering queues, accepting geometry from `Provider[Indexless]`

    enum Primitive
    {
        points    = 1,
        lines     = 2,
        triangles = 3,
    };

    template <typename V, Primitive P>
    class QueueIndexless
    {
        std::vector<V> vertices;
        VertexBuffer<V> vertex_buffer;
        std::size_t vertex_pos = 0; // The amount of non-garbage vertices stored at the beginning of `vertices`. Vertices are inserted at this position.

      public:
        using vertex_t = V;

        static constexpr Primitive primitive = P;

        static constexpr DrawMode draw_mode = []
        {
            switch (primitive)
            {
                case points:    return DrawMode::points;
                case lines:     return DrawMode::lines;
                case triangles: return DrawMode::triangles;
            }
        }();

        QueueIndexless() {}

        QueueIndexless(std::size_t primitive_capacity)
            : vertices(primitive_capacity * int(P)), vertex_buffer(primitive_capacity * int(P), nullptr, dynamic_draw)
        {
            DebugAssert("Invalid render queue capacity.", primitive_capacity > 0);
        }

        explicit operator bool() const
        {
            return bool(vertex_buffer);
        }

        std::size_t VertexCapacity() const
        {
            return vertices.size();
        }
        std::size_t UsedVertexCapacity() const
        {
            return vertex_pos;
        }
        std::size_t RemainingVertexCapacity() const
        {
            return VertexCapacity() - UsedVertexCapacity();
        }

        void Insert(const ProviderIndexless<V> &provider)
        {
            std::size_t vertices_provided = provider.VertexCountFlat();
            if (vertices_provided == 0)
                return;
            DebugAssert("Inserted vertex count is not a multiple of the primitive size.", vertices_provided % int(P) == 0);

            if (RemainingVertexCapacity() == 0)
                Flush();

            std::size_t vertices_inserted = 0;
            while (1)
            {
                std::size_t segment_size = std::min(vertices_provided - vertices_inserted, RemainingVertexCapacity());
                provider.GetVerticesFlat(vertices_inserted, vertices_inserted + segment_size, vertices.data() + vertex_pos);

                vertices_inserted += segment_size;
                vertex_pos += segment_size;

                if (vertices_inserted == vertices_provided)
                    break;
                else
                    Flush();
            }
        }

        void Abort()
        {
            vertex_pos = 0;
        }

        void Flush()
        {
            if (vertex_pos == 0)
                return;
            vertex_buffer.SetDataPart(0, vertex_pos, vertices.data());
            vertex_buffer.Draw(draw_mode, vertex_pos);
            vertex_pos = 0;
        }
    };

    template <typename V, typename I, Primitive P>
    class Queue
    {
        std::vector<V> vertices;
        VertexBuffer<V> vertex_buffer;
        std::size_t vertex_pos = 0; // The amount of non-garbage vertices stored at the beginning of `vertices`. Vertices are inserted at this position.
        std::size_t vertex_pos_uploaded = 0; // The amount of vertices from `vertices` currently uploaded to `vertex_buffer`. Can't be larger than `vertex_pos`.

        std::vector<I> indices; // The amount of non-garbage indices stored at the beginning of `indices`. Vertices are inserted at this position.
        IndexBuffer<I> index_buffer;
        std::size_t index_pos = 0;

        void FlushIndices()
        {
            if (index_pos == 0)
                return;

            if (vertex_pos_uploaded < vertex_pos)
            {
                vertex_buffer.SetDataPart(vertex_pos_uploaded, vertex_pos - vertex_pos_uploaded, vertices.data() + vertex_pos_uploaded);
                vertex_pos_uploaded = vertex_pos;
            }

            index_buffer.SetDataPart(0, index_pos, indices.data());
            index_buffer.Draw(vertex_buffer, draw_mode, index_pos);
            index_pos = 0;
        }

      public:
        using vertex_t = V;
        using index_t = I;

        static constexpr Primitive primitive = P;

        static constexpr DrawMode draw_mode = []
        {
            switch (primitive)
            {
                case points:    return DrawMode::points;
                case lines:     return DrawMode::lines;
                case triangles: return DrawMode::triangles;
            }
        }();

        Queue() {}

        Queue(std::size_t vertex_capacity, std::size_t primitive_index_capacity)
            : vertices(vertex_capacity), vertex_buffer(vertex_capacity, nullptr, dynamic_draw),
            indices(primitive_index_capacity * int(P)), index_buffer(primitive_index_capacity * int(P), nullptr, dynamic_draw)
        {
            DebugAssert("Invalid render queue capacity.", vertex_capacity >= int(P) && primitive_index_capacity > 0);
            DebugAssert("Render queue capacity is too large for this index type.", vertex_capacity - 1 <= std::numeric_limits<index_t>::max());
        }

        explicit operator bool() const
        {
            return bool(vertex_buffer);
        }

        std::size_t VertexCapacity() const
        {
            return vertices.size();
        }
        std::size_t UsedVertexCapacity() const
        {
            return vertex_pos;
        }
        std::size_t RemainingVertexCapacity() const
        {
            return VertexCapacity() - UsedVertexCapacity();
        }

        std::size_t IndexCapacity() const
        {
            return indices.size();
        }
        std::size_t UsedIndexCapacity() const
        {
            return index_pos;
        }
        std::size_t RemainingIndexCapacity() const
        {
            return IndexCapacity() - UsedIndexCapacity();
        }

        void Insert(const Provider<V, I> &provider)
        {
            std::size_t indices_provided = provider.IndexCount();
            if (indices_provided == 0)
                return;
            DebugAssert("Inserted index count is not a multiple of the primitive size.", indices_provided % int(P) == 0);

            std::size_t vertices_provided = provider.VertexCount();
            if (vertices_provided > VertexCapacity())
                Program::Error("Unable to insert geometry into a render queue: too many vertices.");

            if (vertices_provided > RemainingVertexCapacity())
                Flush();

            index_t base_index = vertex_pos;

            provider.GetVertices(0, vertices_provided, vertices.data() + vertex_pos);
            vertex_pos += vertices_provided;

            std::size_t indices_inserted = 0;
            while (1)
            {
                std::size_t segment_size = std::min(indices_provided - indices_inserted, RemainingIndexCapacity());
                provider.GetIndices(indices_inserted, indices_inserted + segment_size, indices.data() + index_pos, base_index);

                indices_inserted += segment_size;
                index_pos += segment_size;

                if (indices_inserted == indices_provided)
                    break;
                else
                    FlushIndices();
            }
        }

        void Abort()
        {
            vertex_pos = 0;
            vertex_pos_uploaded = 0;
            index_pos = 0;
        }

        void Flush()
        {
            FlushIndices();

            vertex_pos = 0;
            vertex_pos_uploaded = 0;
        }
    };
}
