#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <type_traits>
#include <vector>

#include "graphics/index_buffer.h"
#include "graphics/vertex_buffer.h"
#include "program/errors.h"
#include "utils/check.h"
#include "utils/meta.h"

namespace Graphics::Geom
{
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


    template <typename V>
    class ViewIndexless : public ProviderIndexless<V>
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
            source.GetVerticesFlatLow(begin, end, dest);
        }
        void GetVerticesLow(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            source.GetVerticesFlatLow(begin, end, dest);
        }
        void GetIndicesLow(std::size_t begin, std::size_t end, index_t *dest, index_t base_index) const override
        {
            DebugAssert("Vertex index is too large for this type.", (end - begin - 1) + base_index <= std::numeric_limits<index_t>::max());
            std::iota(dest, dest + (end - begin), index_t(begin + base_index));
        }

      public:
        ViewNonIndexBased() {}
        ViewNonIndexBased(const vertex_t *vertex_ptr, std::size_t vertex_count)
            : source(vertex_ptr, vertex_count)
        {}

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
    class View : public Provider<V, I>
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");
        using Base = Provider<V, I>;

      public:
        using typename Base::vertex_t;
        using typename Base::index_t;

      private:
        const index_t *vertex_ptr = nullptr;
        std::size_t vertex_count = 0;
        const index_t *index_ptr = nullptr;
        std::size_t index_count = 0;

      protected:
        void GetVerticesFlatLow(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            source.GetVerticesFlatLow(begin, end, dest);
        }
        void GetVerticesLow(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            std::copy(vertex_ptr + begin, vertex_ptr + end, dest);
        }
        void GetIndicesLow(std::size_t begin, std::size_t end, index_t *dest, index_t base_index) const override
        {
            #error here
            std::tra(index_ptr + begin, index_ptr + end, dest);
        }

      public:
        View() {}
        View(const vertex_t *vertex_ptr, std::size_t vertex_count, const vertex_t *index_ptr, std::size_t index_count)
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




        void GetVerticesFlat(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            DebugAssert("Invalid vertex range.", begin <= end && end <= index_count);
            for (std::size_t i = begin; i < end; i++)
            {
                index_t index = index_ptr[i]
                DebugAssert("Invalid vertex index.", index < vertex_count);
                *dest++ = vertex_ptr[index];
            }
        }

        std::size_t VertexCount() const override
        {
            return vertex_count;
        }
        std::size_t IndexCount() const override
        {
            return index_count;
        }
        void GetVertices(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            source.GetVertices(begin, end, dest);
        }
        void GetIndices(std::size_t begin, std::size_t end, index_t *dest, index_t base_index) const override
        {
            DebugAssert("Invalid index range.", begin <= end && end <= index_count);
            DebugAssert("Vertex index is too large for this type.", (end - begin - 1) + base_index <= std::numeric_limits<index_t>::max());
            std::iota(dest + begin, dest + end, index_t(begin));
        }
    };


    template <typename T, CHECK(std::is_base_of_v<GenericProvider, T>)>
    class AdapterIndexless : public ProviderIndexless<typename T::vertex_t>
    {
        static_assert(!std::is_const_v<T>);

        const T &source;

      public:
        using vertex_t = typename T::vertex_t;

        AdapterIndexless(const T &source) : source(source) {}

        std::size_t VertexCount() const override
        {
            return source.IndexCount();
        }
        const vertex_t &GetVertex(std::size_t pos) const override
        {
            return source.GetVertex(source.GetIndex(pos));
        }
    };

    template <typename T>
    auto AsIndexless(const Provider<V, I> &source)
    {
        return AdapterIndexless<V>(source);
    }

    template <typename I, typename T, CHECK(std::is_base_of_v<GenericProviderIndexless, T>)>
    class AdapterIndexed : public Provider<typename T::vertex_t, I>
    {
        static_assert(!std::is_const_v<T>);

        const T &source;

      public:
        using vertex_t = typename T::vertex_t;
        using index_t = I;

        AdapterIndexed(const T &source) : source(source) {}

        std::size_t VertexCount() const override
        {
            return source.VertexCount();
        }
        std::size_t IndexCount() const override
        {
            return source.VertexCount();
        }
        const vertex_t &GetVertex(std::size_t pos) const override
        {
            return source.GetVertex(pos);
        }
        index_t GetIndex(std::size_t pos) const override
        {
            return index_t(pos);
        }
    };

    template <typename I, typename T>
    auto AsIndexed(const T &source)
    {
        #error fix me and my sibling
        return AdapterIndexed<I, ProviderIndexless<V>>(source);
    }


    template <typename V>
    class DataIndexless
    {
      public:
        using vertex_t = V;

        std::vector<vertex_t> vertices;

        void Insert(const ProviderIndexless<V> &provider)
        {
            std::size_t added_vertex_count = provider.Indexless_VertexCount();
            vertices.reserve(vertices.size() + added_vertex_count);
            for (std::size_t i = 0; i < added_vertex_count; i++)
                vertices.push_back(provider.Indexless_GetVertex(i));
        }

        operator ViewIndexless<V>() const
        {
            return ViewIndexless<V>(vertices.data(), vertices.size());
        }
    };

    template <typename V, typename I>
    class DataIndexed
    {
      public:
        using vertex_t = V;
        using index_t = I;

        std::vector<vertex_t> vertices;
        std::vector<index_t> indices;

        void Insert(const Provider<V, I> &provider)
        {
            std::size_t added_vertex_count = provider.Indexed_VertexCount();
            vertices.reserve(vertices.size() + added_vertex_count);
            for (std::size_t i = 0; i < added_vertex_count; i++)
                vertices.push_back(provider.Indexed_GetVertex(i));

            std::size_t added_index_count = provider.Indexed_IndexCount();
            indices.reserve(indices.size() + added_index_count);
            for (std::size_t i = 0; i < added_index_count; i++)
                indices.push_back(provider.Indexed_GetIndex(i));
        }

        operator View<V, I>() const
        {
            return View<V, I>(vertices.data(), vertices.size(), indices.data(), indices.size());
        }
    };


    template <typename V>
    class BufferIndexless
    {
        DrawMode mode = triangles;
        VertexBuffer<V> vertex_buffer;

      public:
        BufferIndexless() {}
        BufferIndexless(const DataIndexless<V> &vertex_data, DrawMode mode, Usage usage = static_draw)
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
    class BufferIndexed
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");

        DrawMode mode = triangles;
        VertexBuffer<V> vertex_buffer;
        IndexBuffer<I> index_buffer;

      public:
        BufferIndexed() {}
        BufferIndexed(const DataIndexed<V, I> &vertex_data, DrawMode mode, Usage usage = static_draw)
            : mode(mode), vertex_buffer(vertex_data.vertices.size(), vertex_data.vertices.data(), usage), index_buffer(vertex_data.indices.size(), vertex_data.indices.data(), usage)
        {}

        explicit operator bool() const
        {
            return bool(vertex_buffer);
        }

        void Draw()
        {
            index_buffer.Draw(vertex_buffer, mode);
        }
    };


    enum Primitive
    {
        points    = DrawMode::points,
        lines     = DrawMode::lines,
        triangles = DrawMode::triangles,
    };

    template <typename V, Primitive P>
    class QueueIndexless
    {

    };
}
