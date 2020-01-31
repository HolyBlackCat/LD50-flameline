#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include "graphics/index_buffer.h"
#include "graphics/transformations.h"
#include "graphics/vertex_buffer.h"
#include "macros/check.h"
#include "meta/misc.h"
#include "program/errors.h"

namespace Graphics::Geom
{
    // Generic interfaces for vertex containers.

    struct GenericProviderFlat : Meta::with_virtual_destructor<GenericProviderFlat> {};

    template <typename V>
    class ProviderFlat : public GenericProviderFlat
    {
      public:
        using vertex_t = V;

      protected:
        virtual void GetVerticesFlatLow(std::size_t begin, std::size_t end, vertex_t *dest) const
        {
            const vertex_t *vertex_ptr = VertexPointerFlatIfAvailable();
            std::copy(vertex_ptr + begin, vertex_ptr + end, dest);
        }

      public:
        virtual std::size_t VertexCountFlat() const = 0;

        // If no contiguous storage is available, this should return null.
        // You should override either this function, or `GetVerticesFlatLow`.
        virtual const vertex_t *VertexPointerFlatIfAvailable() const {return 0;}

        void GetVerticesFlat(std::size_t begin, std::size_t end, vertex_t *dest) const
        {
            DebugAssert("Invalid vertex range.", begin <= end && end <= VertexCountFlat());
            GetVerticesFlatLow(begin, end, dest);
        }

        void CopyToVector(std::vector<vertex_t> &vertices) const
        {
            if (std::size_t added_vertex_count = VertexCountFlat())
            {
                std::size_t old_vertex_count = vertices.size();
                vertices.resize(vertices.size() + added_vertex_count);
                GetVerticesFlat(0, added_vertex_count, &vertices[old_vertex_count]);
            }
        }
    };

    struct GenericProvider : Meta::with_virtual_destructor<GenericProvider> {};

    template <typename V, typename I>
    class Provider : public GenericProvider
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");

      public:
        using vertex_t = V;
        using index_t = I;

      protected:
        virtual void GetVerticesLow(std::size_t begin, std::size_t end, vertex_t *dest) const
        {
            const vertex_t *vertex_ptr = VertexPointerIfAvailable();
            std::copy(vertex_ptr + begin, vertex_ptr + end, dest);
        }

        virtual void GetIndicesLow(std::size_t begin, std::size_t end, index_t *dest) const
        {
            const index_t *index_ptr = IndexPointerIfAvailable();
            std::copy(index_ptr + begin, index_ptr + end, dest);
        }

      public:
        virtual std::size_t VertexCount() const = 0;
        virtual std::size_t IndexCount() const = 0;

        // If no contiguous storage is available, these should return null.
        // You should override either these function, or `Get*Low` (separately for vertices and indices).
        virtual const vertex_t *VertexPointerIfAvailable() const {return 0;}
        virtual const index_t *IndexPointerIfAvailable() const {return 0;}

        void GetVertices(std::size_t begin, std::size_t end, vertex_t *dest) const
        {
            DebugAssert("Invalid vertex range.", begin <= end && end <= VertexCount());
            GetVerticesLow(begin, end, dest);
        }
        void GetIndices(std::size_t begin, std::size_t end, index_t *dest) const
        {
            DebugAssert("Invalid index range.", begin <= end && end <= IndexCount());
            GetIndicesLow(begin, end, dest);
        }

        void CopyToVectors(std::vector<vertex_t> &vertices, std::vector<index_t> &indices)
        {
            std::size_t old_vertex_count = vertices.size();

            if (std::size_t added_vertex_count = VertexCount())
            {
                vertices.resize(vertices.size() + added_vertex_count);
                GetVertices(0, added_vertex_count, &vertices[old_vertex_count]);
            }

            if (std::size_t added_index_count = IndexCount())
            {
                std::size_t old_index_count = indices.size();
                indices.resize(indices.size() + added_index_count);
                GetIndices(0, added_index_count, &indices[old_index_count]);

                for (std::size_t i = 0; i < added_index_count; i++)
                {
                    index_t &this_index = indices[i + old_index_count];
                    DebugAssert("Vertex index overflow.", index_t(this_index + old_vertex_count) >= this_index);
                    this_index += old_vertex_count;
                }
            }
        }
    };


    // Vertex container transformations.
    // Note that the result of `transformation * container` maintains a reference to `container`.
    // Beware of dangling references!

    template <typename P, typename T, CHECK(std::is_base_of_v<GenericProviderFlat, P>),
        CHECK_EXPR(std::declval<const T &>() * std::declval<const typename P::vertex_t &>())>
    class TransformedProviderFlat : public ProviderFlat<typename P::vertex_t>
    {
        using Base = ProviderFlat<typename P::vertex_t>;

        const Base &src_provider;
        T transformation;

      public:
        using typename Base::vertex_t;

        TransformedProviderFlat(const P &provider, const T &transformation)
            : src_provider(provider), transformation(transformation) {}

        std::size_t VertexCountFlat() const override
        {
            return src_provider.VertexCountFlat();
        }

      protected:
        void GetVerticesFlatLow(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            src_provider.GetVerticesFlat(begin, end, dest);
            for (std::size_t i = 0; i < end - begin; i++)
                dest[i] = transformation * dest[i];
        }
    };

    template <typename P, typename T, CHECK(std::is_base_of_v<GenericProvider, P>),
        CHECK_EXPR(std::declval<const T &>() * std::declval<const typename P::vertex_t &>())>
    class TransformedProvider : public Provider<typename P::vertex_t, typename P::index_t>
    {
        using Base = Provider<typename P::vertex_t, typename P::index_t>;

        const Base &src_provider;
        T transformation;

      public:
        using typename Base::vertex_t;
        using typename Base::index_t;

        TransformedProvider(const P &provider, const T &transformation)
            : src_provider(provider), transformation(transformation) {}

        std::size_t VertexCount() const override
        {
            return src_provider.VertexCount();
        }
        std::size_t IndexCount() const override
        {
            return src_provider.IndexCount();
        }

        const index_t *IndexPointerIfAvailable() const override
        {
            // Note that we don't expose `VertexPointerIfAvailable()` in the same way, since we modify the vertices.
            return src_provider.IndexPointerIfAvailable();
        }

      protected:
        void GetVerticesLow(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            src_provider.GetVertices(begin, end, dest);
            for (std::size_t i = 0; i < end - begin; i++)
                dest[i] = transformation * dest[i];
        }
        void GetIndicesLow(std::size_t begin, std::size_t end, index_t *dest) const override
        {
            src_provider.GetIndices(begin, end, dest);
        }
    };

    template <typename T, typename P>
    TransformedProviderFlat<P, T> operator*(const T &transformation, const P &provider)
    {
        return {provider, transformation};
    }

    template <typename T, typename P>
    TransformedProvider<P, T> operator*(const T &transformation, const P &provider)
    {
        return {provider, transformation};
    }


    // References to geometry providers, intended to serve as function providers.
    // Both indexless and indexed views can refer to indexless and indexed providers, even if index types are different.

    template <typename T>
    class DataView
    {
        // Internal. `View[Flat]` use this call to refer to vertex and index arrays.
      public:
        using type = T;

        using get_elements_func_t = void(const void *target, std::size_t begin, std::size_t end, T *dest);

      private:
        // The class can be in three different states:
        // * A null state.
        // * Pointing to a contiguous storage (size is set, target is null, pointer is non-null, storage may or may not be null).
        // * Pointing to arbitrary object (size is set, target is set, pointer is null, storage is null, get_elements is non-null).

        std::size_t size = 0; // The amount of elements.
        const void *target = 0; // Pointer to an arbitrary user object, passed to `get_elements`.
        const type *pointer = 0; // Pointer to contiguous storage (possibly `storage`), or null.
        std::unique_ptr<type[]> storage; // Internal storage, may or may not be used.
        get_elements_func_t *get_elements = 0; // A callback to get elements from `target`.

      public:
        DataView() {}

        [[nodiscard]] static DataView ToContiguousStorage(std::size_t size, const type *pointer)
        {
            DataView ret;
            ret.size = size;
            ret.pointer = pointer;
            return ret;
        }

        [[nodiscard]] static DataView ToArbitraryObject(std::size_t size, const void *target, get_elements_func_t *get_elements)
        {
            DataView ret;
            ret.size = size;
            ret.target = target;
            ret.get_elements = get_elements;
            return ret;
        }

        explicit operator bool() const
        {
            return bool(pointer) || bool(get_elements);
        }

        bool IsContiguous() const
        {
            DebugAssert("Attempt to use a null `DataView` instance.", *this);
            return bool(pointer);
        }

        void MakeContiguousIfNeeded()
        {
            DebugAssert("Attempt to use a null `DataView` instance.", *this);

            if (IsContiguous())
                return;

            storage = std::make_unique<type[]>(size);
            pointer = storage.get();

            get_elements(target, 0, size, storage.get());
            get_elements = 0;
            target = 0;
        }

        std::size_t Size() const
        {
            DebugAssert("Attempt to use a null `DataView` instance.", *this);
            return size;
        }

        // Can return null if no contiguous storage is available.
        const type *PointerIfAvailable() const
        {
            DebugAssert("Attempt to use a null `DataView` instance.", *this);
            return pointer;
        }

        // Always returns a non-null pointer.
        // If necessary, copies data to an internal storage first.
        const type *Pointer() const
        {
            DebugAssert("Attempt to use a null `DataView` instance.", *this);
            MakeContiguousIfNeeded();
            return pointer;
        }

        void GetElements(std::size_t begin, std::size_t end, type *dest) const
        {
            DebugAssert("Attempt to use a null `DataView` instance.", *this);

            if (pointer)
                std::copy(pointer + begin, pointer + end, dest);
            else
                get_elements(target, begin, end, dest);
        }
    };

    template <typename V>
    class ViewFlat : public ProviderFlat<V>
    {
      public:
        using typename ProviderFlat<V>::vertex_t;

      private:
        DataView<vertex_t> vertex_view;

      public:
        ViewFlat(const ProviderFlat<vertex_t> &provider)
        {
            if (provider.VertexPointerFlatIfAvailable())
            {
                vertex_view = DataView<vertex_t>::ToContiguousStorage(provider.VertexCountFlat(), provider.VertexPointerFlatIfAvailable());
            }
            else
            {
                vertex_view = DataView<vertex_t>::ToArbitraryObject(provider.VertexCountFlat(), &provider,
                    [](const void *provider_ptr, std::size_t begin, std::size_t end, vertex_t *dest)
                    {
                        const auto &provider = *static_cast<const ProviderFlat<vertex_t> *>(provider_ptr);
                        provider.GetVerticesFlat(begin, end, dest);
                    });
            }
        }

        template <typename T, CHECK(std::is_base_of_v<GenericProvider, T> && std::is_same_v<typename T::vertex_t, vertex_t>)>
        ViewFlat(const T &provider_obj)
        {
            const Provider<typename T::vertex_t, typename T::index_t> &provider = provider_obj;
            using index_t = typename T::index_t;

            vertex_view = DataView<vertex_t>::ToArbitraryObject(provider.IndexCount(), &provider,
                Meta::with_cexpr_flags(provider.VertexPointerIfAvailable(), provider.IndexPointerIfAvailable())
                >> [](auto contiguous_vertices, auto contiguous_indices)
                {
                    constexpr bool have_vertex_ptr = contiguous_vertices;
                    constexpr bool have_index_ptr = contiguous_indices;

                    return +[](const void *provider_ptr, std::size_t begin, std::size_t end, vertex_t *dest)
                    {
                        const auto &provider = *static_cast<const T *>(provider_ptr);

                        const vertex_t *vertex_ptr = provider.VertexPointerIfAvailable();
                        const index_t *index_ptr = provider.IndexPointerIfAvailable();

                        for (std::size_t i = begin; i < end; i++)
                        {
                            index_t index;
                            if constexpr (have_index_ptr)
                                index = index_ptr[i];
                            else
                                provider.GetIndices(i, i+1, &index);

                            DebugAssert("Invalid vertex index.", index < provider.VertexCount());

                            if constexpr (have_vertex_ptr)
                                *dest = vertex_ptr[index];
                            else
                                provider.GetVertices(index, index+1, dest);

                            dest++;
                        }
                    };
                });
        }

              DataView<vertex_t> &Vertices()       {return vertex_view;}
        const DataView<vertex_t> &Vertices() const {return vertex_view;}

        std::size_t VertexCountFlat() const override
        {
            return vertex_view.Size();
        }

        // Call `Vertices().MakeContiguousIfNeeded()` to force this to return a non-null pointer.
        const vertex_t *VertexPointerFlatIfAvailable() const override
        {
            return vertex_view.PointerIfAvailable();
        }

      protected:
        void GetVerticesFlatLow(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            vertex_view.GetElements(begin, end, dest);
        }
    };

    template <typename V, typename I>
    class View : public Provider<V, I>
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");

      public:
        using typename Provider<V, I>::vertex_t;
        using typename Provider<V, I>::index_t;

      private:
        DataView<vertex_t> vertex_view;
        DataView<index_t> index_view;

      public:
        View(const ProviderFlat<vertex_t> &provider)
        {
            if (provider.VertexPointerFlatIfAvailable())
            {
                vertex_view = DataView<vertex_t>::ToContiguousStorage(provider.VertexCountFlat(), provider.VertexPointerFlatIfAvailable());
            }
            else
            {
                vertex_view = DataView<vertex_t>::ToArbitraryObject(provider.VertexCountFlat(), &provider,
                    [](const void *provider_ptr, std::size_t begin, std::size_t end, vertex_t *dest)
                    {
                        const auto &provider = *static_cast<const ProviderFlat<vertex_t> *>(provider_ptr);
                        provider.GetVerticesFlat(begin, end, dest);
                    });
            }

            index_view = DataView<index_t>::ToArbitraryObject(provider.VertexCountFlat(), nullptr,
                [](const void *, std::size_t begin, std::size_t end, index_t *dest)
                {
                    DebugAssert("Vertex index is too large for this type.", end == 0 || end - 1 <= std::numeric_limits<index_t>::max());
                    std::iota(dest, dest + end - begin, index_t(begin));
                });
        }

        View(const Provider<vertex_t, index_t> &provider)
        {
            if (provider.VertexPointerIfAvailable())
            {
                vertex_view = DataView<vertex_t>::ToContiguousStorage(provider.VertexCount(), provider.VertexPointerIfAvailable());
            }
            else
            {
                vertex_view = DataView<vertex_t>::ToArbitraryObject(provider.VertexCount(), &provider,
                    [](const void *provider_ptr, std::size_t begin, std::size_t end, vertex_t *dest)
                    {
                        const auto &provider = *static_cast<const Provider<vertex_t, index_t> *>(provider_ptr);
                        provider.GetVertices(begin, end, dest);
                    });
            }

            if (provider.IndexPointerIfAvailable())
            {
                index_view = DataView<index_t>::ToContiguousStorage(provider.IndexCount(), provider.IndexPointerIfAvailable());
            }
            else
            {
                index_view = DataView<index_t>::ToArbitraryObject(provider.IndexCount(), &provider,
                    [](const void *provider_ptr, std::size_t begin, std::size_t end, index_t *dest)
                    {
                        const auto &provider = *static_cast<const Provider<vertex_t, index_t> *>(provider_ptr);
                        provider.GetIndices(begin, end, dest);
                    });
            }
        }

        template <typename T, CHECK(std::is_base_of_v<GenericProvider, T> &&
                                    std::is_same_v<typename T::vertex_t, vertex_t> &&
                                    /*sic*/!std::is_same_v<typename T::index_t, index_t>)>
        View(const T &provider_obj)
        {
            const Provider<typename T::vertex_t, typename T::index_t> &provider = provider_obj;

            if (provider.VertexPointerIfAvailable())
            {
                vertex_view = DataView<vertex_t>::ToContiguousStorage(provider.VertexCount(), provider.VertexPointerIfAvailable());
            }
            else
            {
                vertex_view = DataView<vertex_t>::ToArbitraryObject(provider.VertexCount(), &provider,
                    [](const void *provider_ptr, std::size_t begin, std::size_t end, vertex_t *dest)
                    {
                        const auto &provider = *static_cast<const T *>(provider_ptr);
                        provider.GetVertices(begin, end, dest);
                    });
            }

            index_view = DataView<index_t>::ToArbitraryObject(provider.IndexCount(), &provider,
                [](const void *provider_ptr, std::size_t begin, std::size_t end, index_t *dest)
                {
                    const auto &provider = *static_cast<const T *>(provider_ptr);

                    for (std::size_t i = begin; i < end; i++)
                    {
                        typename T::index_t index;
                        provider.GetIndices(i, i+1, &index);

                        if constexpr (sizeof index > sizeof(index_t))
                            DebugAssert("Vertex index is too large for this type.", index <= std::numeric_limits<index_t>::max());

                        *dest++ = index;
                    }
                });
        }

              DataView<vertex_t> &Vertices()       {return vertex_view;}
        const DataView<vertex_t> &Vertices() const {return vertex_view;}
              DataView<index_t> &Indices()       {return index_view;}
        const DataView<index_t> &Indices() const {return index_view;}

        std::size_t VertexCount() const override
        {
            return vertex_view.Size();
        }
        std::size_t IndexCount() const override
        {
            return index_view.Size();
        }

        // Call `.MakeContiguousIfNeeded()` on `Vertices()` or `Indices()` to force these to return non-null pointers.
        const vertex_t *VertexPointerIfAvailable() const override
        {
            return vertex_view.PointerIfAvailable();
        }
        const index_t *IndexPointerIfAvailable() const override
        {
            return index_view.PointerIfAvailable();
        }

      protected:
        void GetVerticesLow(std::size_t begin, std::size_t end, vertex_t *dest) const override
        {
            vertex_view.GetElements(begin, end, dest);
        }
        void GetIndicesLow(std::size_t begin, std::size_t end, index_t *dest) const override
        {
            index_view.GetElements(begin, end, dest);
        }
    };


    // Wrappers around pointers to existing contiguous storage, implementing `Provider[Flat]`.

    template <typename T>
    using contiguous_container_elem_t = std::remove_pointer_t<decltype(std::declval<T &>().data())>;

    template <typename T>
    class DataRef
    {
        // Internal. A simple span-like class.
      public:
        using type = T;

      private:
        const type *pointer = 0;
        std::size_t size = 0;

      public:
        constexpr DataRef() {}

        constexpr DataRef(const type *pointer, std::size_t size)
            : pointer(pointer), size(size)
        {}

        constexpr DataRef(std::initializer_list<type> init_list)
            : pointer(&*init_list.begin()), size(init_list.size())
        {}

        template <std::size_t N>
        constexpr DataRef(const type (&array)[N])
            : pointer(array), size(N)
        {}

        template <typename C, CHECK_TYPE(contiguous_container_elem_t<C>)>
        constexpr DataRef(const C &container)
            : pointer(container.data()), size(container.size())
        {}

        explicit operator bool() const
        {
            return bool(pointer);
        }

        const type *Pointer() const
        {
            return pointer;
        }
        std::size_t Size() const
        {
            return size;
        }
    };

    template <typename V>
    class RefFlat : public ProviderFlat<V>
    {
        using Base = ProviderFlat<V>;

      public:
        using typename Base::vertex_t;

      private:
        DataRef<vertex_t> vertex_ref;

      public:
        RefFlat() {}
        RefFlat(DataRef<vertex_t> vertex_ref) : vertex_ref(vertex_ref) {}

        std::size_t VertexCountFlat() const override
        {
            return vertex_ref.Size();
        }

        const vertex_t *VertexPointerFlatIfAvailable() const override
        {
            return vertex_ref.Pointer();
        }
    };

    template <typename V, typename I>
    class Ref : public Provider<V, I>
    {
        static_assert(is_valid_index_type_v<I>, "Invalid index type.");
        using Base = Provider<V, I>;

      public:
        using typename Base::vertex_t;
        using typename Base::index_t;

      private:
        DataRef<vertex_t> vertex_ref;
        DataRef<index_t> index_ref;

      public:
        Ref() {}
        Ref(DataRef<vertex_t> vertex_ref, DataRef<index_t> index_ref) : vertex_ref(vertex_ref), index_ref(index_ref) {}

        std::size_t VertexCount() const override
        {
            return vertex_ref.Size();
        }
        std::size_t IndexCount() const override
        {
            return index_ref.Size();
        }

        const vertex_t *VertexPointerIfAvailable() const override
        {
            return vertex_ref.Pointer();
        }
        const index_t *IndexPointerIfAvailable() const override
        {
            return index_ref.Pointer();
        }
    };


    // Wrappers for vectors or arrays, implementing `Provider[Flat]`.

    template <typename VC>
    class DataFlat : public ProviderFlat<contiguous_container_elem_t<VC>>
    {
        using Base = ProviderFlat<contiguous_container_elem_t<VC>>;

      public:
        using typename Base::vertex_t;

        using vertex_container_t = VC;
        vertex_container_t vertices;

        DataFlat() {}
        DataFlat(vertex_container_t vertices) : vertices(std::move(vertices)) {}

        DataFlat(ViewFlat<vertex_t> view)
        {
            Insert(std::move(view));
        }

        // Only work if the underlying container is a vector.
        void Insert(ViewFlat<vertex_t> view)
        {
            view.CopyToVector(vertices);
        }
        DataFlat &operator<<(ViewFlat<vertex_t> view)
        {
            Insert(std::move(view));
            return *this;
        }

        std::size_t VertexCountFlat() const override
        {
            return vertices.size();
        }

        const vertex_t *VertexPointerFlatIfAvailable() const override
        {
            return vertices.data();
        }
    };

    template <typename VC, typename IC>
    class Data : public Provider<contiguous_container_elem_t<VC>, contiguous_container_elem_t<IC>>
    {
        using Base = Provider<contiguous_container_elem_t<VC>, contiguous_container_elem_t<IC>>;

      public:
        using typename Base::vertex_t;
        using typename Base::index_t;

        using vertex_container_t = VC;
        using index_container_t = IC;
        vertex_container_t vertices;
        index_container_t indices;

        Data() {}
        Data(vertex_container_t vertices, index_container_t indices) : vertices(std::move(vertices)), indices(std::move(indices)) {}

        Data(View<vertex_t, index_t> view)
        {
            Insert(std::move(view));
        }

        // Only work if the underlying containers are vectors.
        void Insert(View<vertex_t, index_t> view)
        {
            view.CopyToVectors(vertices, indices);
        }
        Data &operator<<(View<vertex_t, index_t> view)
        {
            Insert(std::move(view));
            return *this;
        }

        std::size_t VertexCount() const override
        {
            return vertices.size();
        }
        std::size_t IndexCount() const override
        {
            return indices.size();
        }

        const vertex_t *VertexPointerIfAvailable() const override
        {
            return vertices.data();
        }
        const index_t *IndexPointerIfAvailable() const override
        {
            return indices.data();
        }
    };


    // Drawable buffers that can be constructed from `ArrayProvider[Flat]`.

    template <typename V>
    class BufferFlat
    {
        DrawMode mode = triangles;
        VertexBuffer<V> vertex_buffer;

      public:
        using vertex_t = V;

        BufferFlat() {}
        BufferFlat(DrawMode mode, ViewFlat<V> view, Usage usage = static_draw)
            : mode(mode)
        {
            view.Vertices().MakeContiguousIfNeeded();
            vertex_buffer = VertexBuffer<V>(view.VertexCountFlat(), view.VertexPointerFlatIfAvailable(), usage);
        }

        explicit operator bool() const
        {
            return bool(vertex_buffer);
        }

        DrawMode GetDrawMode() const {return mode;}
        void SetDrawMode(DrawMode new_mode) {mode = new_mode;}

              VertexBuffer<V> &Vertices()       {return vertex_buffer;}
        const VertexBuffer<V> &Vertices() const {return vertex_buffer;}

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
        Buffer(DrawMode mode, View<V, I> view, Usage usage = static_draw)
            : mode(mode)
        {
            view.Vertices().MakeContiguousIfNeeded();
            view.Indices().MakeContiguousIfNeeded();

            vertex_buffer = VertexBuffer<V>(view.VertexCount(), view.VertexPointerIfAvailable(), usage);
            index_buffer = IndexBuffer<I>(view.IndexCount(), view.IndexPointerIfAvailable(), usage);
        }

        explicit operator bool() const
        {
            return bool(vertex_buffer);
        }

        DrawMode GetDrawMode() const {return mode;}
        void SetDrawMode(DrawMode new_mode) {mode = new_mode;}

              VertexBuffer<V> &Vertices()       {return vertex_buffer;}
        const VertexBuffer<V> &Vertices() const {return vertex_buffer;}

              IndexBuffer<V> &Indices()       {return index_buffer;}
        const IndexBuffer<V> &Indices() const {return index_buffer;}

        void Draw() const
        {
            index_buffer.Draw(vertex_buffer, mode);
        }
    };


    // Rendering queues, accepting geometry from `Provider[Flat]`

    enum Primitive
    {
        points    = 1,
        lines     = 2,
        triangles = 3,
    };

    inline constexpr DrawMode PrimitiveToDrawMode(Primitive primitive)
    {
        switch (primitive)
        {
            case points:    return DrawMode::points;
            case lines:     return DrawMode::lines;
            case triangles: return DrawMode::triangles;
        }

        return DrawMode::points;
    }

    template <typename V, Primitive P>
    class QueueFlat
    {
        std::vector<V> vertices;
        VertexBuffer<V> vertex_buffer;
        std::size_t vertex_pos = 0; // The amount of non-garbage vertices stored at the beginning of `vertices`. Vertices are inserted at this position.

      public:
        using vertex_t = V;

        static constexpr Primitive primitive = P;

        QueueFlat() {}

        QueueFlat(std::size_t primitive_capacity)
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

        void Insert(ViewFlat<V> view)
        {
            std::size_t vertices_provided = view.VertexCountFlat();
            if (vertices_provided == 0)
                return;
            DebugAssert("Inserted vertex count is not a multiple of the primitive size.", vertices_provided % int(P) == 0);

            if (RemainingVertexCapacity() == 0)
                Flush();

            std::size_t vertices_inserted = 0;
            while (1)
            {
                std::size_t segment_size = std::min(vertices_provided - vertices_inserted, RemainingVertexCapacity());
                view.GetVerticesFlat(vertices_inserted, vertices_inserted + segment_size, vertices.data() + vertex_pos);

                vertices_inserted += segment_size;
                vertex_pos += segment_size;

                if (vertices_inserted == vertices_provided)
                    break;
                else
                    Flush();
            }
        }
        QueueFlat &operator<<(ViewFlat<vertex_t> view)
        {
            Insert(std::move(view));
            return *this;
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
            vertex_buffer.Draw(PrimitiveToDrawMode(primitive), vertex_pos);
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
            index_buffer.Draw(vertex_buffer, PrimitiveToDrawMode(primitive), index_pos);
            index_pos = 0;
        }

      public:
        using vertex_t = V;
        using index_t = I;

        static constexpr Primitive primitive = P;

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

        void Insert(View<V, I> view)
        {
            std::size_t indices_provided = view.IndexCount();
            if (indices_provided == 0)
                return;
            DebugAssert("Inserted index count is not a multiple of the primitive size.", indices_provided % int(P) == 0);

            std::size_t vertices_provided = view.VertexCount();
            if (vertices_provided > VertexCapacity())
                Program::Error("Unable to insert geometry into a render queue: too many vertices.");

            if (vertices_provided > RemainingVertexCapacity())
                Flush();

            index_t base_index = vertex_pos;

            view.GetVertices(0, vertices_provided, vertices.data() + vertex_pos);
            vertex_pos += vertices_provided;

            std::size_t indices_inserted = 0;
            while (1)
            {
                std::size_t segment_size = std::min(indices_provided - indices_inserted, RemainingIndexCapacity());
                view.GetIndices(indices_inserted, indices_inserted + segment_size, indices.data() + index_pos);
                for (std::size_t i = 0; i < segment_size; i++)
                    indices[index_pos + i] += base_index;

                indices_inserted += segment_size;
                index_pos += segment_size;

                if (indices_inserted == indices_provided)
                    break;
                else
                    FlushIndices();
            }
        }
        Queue &operator<<(View<vertex_t, index_t> view)
        {
            Insert(std::move(view));
            return *this;
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
