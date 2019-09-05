#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <vector>

#include "graphics/index_buffer.h"
#include "graphics/vertex_buffer.h"
#include "program/errors.h"
#include "utils/meta.h"

namespace Graphics
{
    template <typename T> inline constexpr bool is_valid_index_type_v = std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t> || std::is_same_v<T, std::uint32_t>;

    template <typename V>
    class GeometryProviderIndexless : Meta::with_virtual_destructor<GeometryProviderIndexless<V>>
    {
      public:
        using vertex_t = V;

        virtual explicit operator bool() const = 0;

        virtual std::size_t Indexless_VertexCount() const = 0;
        virtual const vertex_t &Indexless_GetVertex(std::size_t pos) const = 0;
    };

    template <typename V, typename I>
    class GeometryProviderIndexed : Meta::with_virtual_destructor<GeometryProviderIndexed<V, I>>
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");

      public:
        using vertex_t = V;
        using index_t = I;

        virtual explicit operator bool() const = 0;

        virtual std::size_t Indexed_VertexCount() const = 0;
        virtual std::size_t Indexed_IndexCount() const = 0;
        virtual const vertex_t &Indexed_GetVertex(std::size_t pos) const = 0;
        virtual index_t Indexed_GetIndex(std::size_t pos) const = 0;
    };


    template <typename V, typename I>
    class GeometryProvider final : public GeometryProviderIndexless<V>, public GeometryProviderIndexed<V, I>
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");

      public:
        using vertex_t = V;
        using index_t = I;

      private:
        const vertex_t *vertex_ptr = nullptr;
        std::size_t vertex_count = 0;

        const index_t *index_ptr = nullptr; // Can be null if the data is indexless.
        std::size_t index_count = 0; // Ignored if `index_ptr` is null.

        bool IsIndexless() const
        {
            return index_ptr == nullptr;
        }

      public:
        GeometryProvider() {}

        // Indexless constructor
        GeometryProvider(const vertex_t *vertex_ptr, std::size_t vertex_count)
            : vertex_ptr(vertex_ptr), vertex_count(vertex_count) {}

        // Indexed constructor
        GeometryProvider(const vertex_t *vertex_ptr, std::size_t vertex_count, const index_t *index_ptr, std::size_t index_count)
            : vertex_ptr(vertex_ptr), vertex_count(vertex_count), index_ptr(index_ptr), index_count(index_count) {}

        // Indexless interface
        std::size_t Indexless_VertexCount() const override
        {
            return IsIndexless() ? vertex_count : index_count;
        }
        const vertex_t &Indexless_GetVertex(std::size_t pos) const override
        {
            if (IsIndexless())
            {
                DebugAssert("Vertex position is out of range.", pos < vertex_count);
                return vertex_ptr[pos];
            }
            else
            {
                DebugAssert("Vertex position is out of range.", pos < index_count);
                DebugAssert("Vertex index is out of range.", index_ptr[pos] < vertex_count);
                return vertex_ptr[index_ptr[pos]];
            }
        }

        // Indexed interface
        std::size_t Indexed_VertexCount() const override
        {
            return vertex_count;
        }
        std::size_t Indexed_IndexCount() const override
        {
            return IsIndexless() ? vertex_count : index_count;
        }
        const vertex_t &Indexed_GetVertex(std::size_t pos) const override
        {
            DebugAssert("Vertex position is out of range.", pos < vertex_count);
            return vertex_ptr[pos];
        }
        index_t Indexed_GetIndex(std::size_t pos) const override
        {
            if (IsIndexless())
            {
                DebugAssert("Vertex index position is out of range.", pos < vertex_count);
                return pos;
            }
            else
            {
                DebugAssert("Vertex index position is out of range.", pos < index_count);
                DebugAssert("Vertex index is out of range.", index_ptr[pos] < vertex_count);
                return index_ptr[pos];
            }
        }
    };


    template <typename V>
    class GeometryConsumerIndexless : Meta::with_virtual_destructor<GeometryConsumerIndexless<V>>
    {
      public:
        using vertex_t = V;
        using provider_t = GeometryProviderIndexless<V>;

        virtual void Insert(const provider_t &provider) = 0;
        virtual void Flush() {}

        GeometryConsumerIndexless &operator<<(const provider_t &provider)
        {
            Insert(provider);
            return *this;
        }
    };

    template <typename V, typename I>
    class GeometryConsumerIndexed : Meta::with_virtual_destructor<GeometryConsumerIndexed<V, I>>
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");

      public:
        using vertex_t = V;
        using index_t = I;
        using provider_t = GeometryProviderIndexed<V, I>;

        virtual void Insert(const provider_t &provider) = 0;
        virtual void Flush() {}

        GeometryConsumerIndexed &operator<<(const provider_t &provider)
        {
            Insert(provider);
            return *this;
        }
    };


    template <typename V>
    class VertexDataIndexless : public GeometryConsumerIndexless<V>
    {
        using base = GeometryConsumerIndexless<V>;

      public:
        using typename base::vertex_t;
        using typename base::index_t;
        using typename base::provider_t;

        std::vector<vertex_t> vertices;

        void Insert(const provider_t &provider) override
        {
            DebugAssert(provider, "Null vertex data provider.");

            std::size_t added_vertex_count = provider.Indexless_VertexCount();
            vertices.reserve(vertices.size() + added_vertex_count);
            for (std::size_t i = 0; i < added_vertex_count; i++)
                vertices.push_back(provider.Indexless_GetVertex(i));
        }
    };

    template <typename V, typename I>
    class VertexDataIndexed : public GeometryConsumerIndexed<V, I>
    {
        using base = GeometryConsumerIndexed<V, I>;

      public:
        using typename base::vertex_t;
        using typename base::index_t;
        using typename base::provider_t;

        std::vector<vertex_t> vertices;
        std::vector<index_t> indices;

        void Insert(const provider_t &provider) override
        {
            DebugAssert(provider, "Null vertex data provider.");

            std::size_t added_vertex_count = provider.Indexed_VertexCount();
            vertices.reserve(vertices.size() + added_vertex_count);
            for (std::size_t i = 0; i < added_vertex_count; i++)
                vertices.push_back(provider.Indexed_GetVertex(i));

            std::size_t added_index_count = provider.Indexed_IndexCount();
            indices.reserve(indices.size() + added_index_count);
            for (std::size_t i = 0; i < added_index_count; i++)
                indices.push_back(provider.Indexed_GetIndex(i));
        }
    };


    template <typename V>
    class GeometryIndexless
    {
        DrawMode mode = triangles;
        VertexBuffer<V> vertex_buffer;

      public:
        GeometryIndexless() {}
        GeometryIndexless(const VertexDataIndexless<V> &vertex_data, DrawMode mode, Usage usage = static_draw)
            : mode(mode), vertex_buffer(vertex_data.vertices.size(), vertex_data.vertices.data(), usage)
        {}

        explicit operator bool() const
        {
            return bool(vertex_buffer);
        }

        void Draw()
        {
            vertex_buffer.Draw(mode);
        }
    };

    template <typename V, typename I>
    class GeometryIndexed
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");

        DrawMode mode = triangles;
        VertexBuffer<V> vertex_buffer;
        IndexBuffer<I> index_buffer;

      public:
        GeometryIndexed() {}
        GeometryIndexed(const VertexDataIndexed<V, I> &vertex_data, DrawMode mode, Usage usage = static_draw)
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
