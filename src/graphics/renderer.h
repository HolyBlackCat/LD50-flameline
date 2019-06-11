#pragma once

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <utility>

#include "graphics/index_buffer.h"
#include "graphics/vertex_buffer.h"
#include "program/errors.h"
#include "utils/adjust.h"
#include "utils/meta.h"
#include "utils/poly_storage.h"

namespace Graphics
{
    template <typename V, typename I = std::uint16_t>
    class Renderer : Meta::with_virtual_destructor<Renderer<V, I>>
    {
      public:
        using vertex_t = V;
        using index_t = I;

        class Operation;
        class IndexedOperation;

        virtual bool HaveEnoughStorage(std::size_t vertex_count, std::size_t index_count) const = 0;

        virtual Operation BeginOperation(std::size_t vertex_count) = 0;

        // `Operation` calls this automatically in the destructor (if non-null).
        // This function has to end with `op = {};`.
        virtual void EndOperation(Operation &op) = 0;

        virtual IndexedOperation BeginOperationIndexed(std::size_t vertex_count, std::size_t index_count) = 0;

        // `IndexedOperation` calls this automatically in the destructor (if non-null).
        // This function has to end with `op = {};`.
        virtual void EndOperation(IndexedOperation &op) = 0;

        virtual void Flush() {}

        virtual void Cleanup() {} // Free temporary memory to reduce memory usage.

        class Operation
        {
          protected:
            struct Data
            {
                Renderer *renderer = 0;
                vertex_t *vertex_pointer = 0;
                std::size_t vertex_count = 0;
            };
            Meta::reset_on_move<Data> data;

          public:
            Operation() {}

            static Operation Construct(Renderer *renderer, vertex_t *vertex_pointer, std::size_t vertex_count) // For internal use.
            {
                return adjust(Operation{}, data->renderer = renderer, data->vertex_pointer = vertex_pointer, data->vertex_count = vertex_count);
            }

            Operation(Operation &&) = default;
            Operation &operator=(Operation &&) = default;

            ~Operation()
            {
                if (data->renderer)
                    data->renderer->EndOperation(*this);
            }

            // Note that the functions here are const-qualified.

            vertex_t *vertex_pointer() const
            {
                return data->vertex_pointer;
            }

            std::size_t vertex_count() const
            {
                return data->vertex_count;
            }

            vertex_t &vertex(std::size_t i) const
            {
                DebugAssert("Renderer: index is out of range.", i < vertex_count());
                return data->vertex_pointer[i];
            }

            void copy_vertices_from(const vertex_t *source) const
            {
                std::copy_n(source, vertex_count(), vertex_pointer());
            }
        };

        class IndexedOperation : public Operation
        {
          protected:
            struct Data
            {
                index_t *index_pointer = 0;
                std::size_t index_count = 0;
                index_t i_base = 0;
            };
            Meta::reset_on_move<Data> data;

          public:
            IndexedOperation() {}

            static IndexedOperation Construct(Renderer *renderer, vertex_t *vertex_pointer, std::size_t vertex_count, index_t *index_pointer, std::size_t index_count, index_t i_base) // For internal use.
            {
                IndexedOperation ret;
                static_cast<Operation &>(ret) = Operation::Construct(renderer, vertex_pointer, vertex_count);
                ret.data->index_pointer = index_pointer;
                ret.data->index_count = index_count;
                ret.data->i_base = i_base;
                return ret;
            }

            IndexedOperation(IndexedOperation &&) = default;
            IndexedOperation &operator=(IndexedOperation &&) = default;

            ~IndexedOperation()
            {
                if (Operation::data->renderer)
                    Operation::data->renderer->EndOperation(*this);
            }

            // Note that the functions here are const-qualified.

            std::size_t index_count() const
            {
                return data->index_count;
            }

            void set_index(std::size_t i, index_t index) const
            {
                DebugAssert("Renderer: index is out of range.", i < index_count());
                DebugAssert("Renderer: vertex index is invalid.", index < this->vertex_count());
                data->index_pointer[i] = index + data->i_base;
            }

            void copy_indices_from(const index_t *source) const
            {
                for (std::size_t i = 0; i < index_count(); i++)
                    set_index(i, source[i]);
            }
        };

        void AddVertex(const vertex_t &vertex)
        {
            Operation op = BeginOperation(1);
            op.vertex(0) = vertex;
        }

        void AddVertices(const vertex_t *vertices, std::size_t vertex_count)
        {
            Operation op = BeginOperation(vertex_count);
            op.copy_vertices_from(vertices);
        }

        void AddVerticesAndIndices(const vertex_t *vertices, std::size_t vertex_count, const index_t *indices, std::size_t index_count)
        {
            IndexedOperation op = BeginOperationIndexed(vertex_count, index_count);
            op.copy_vertices_from(vertices);
            op.copy_indices_from(indices);
        }
    };

    template <typename V, typename I = std::uint16_t> class ArrayRenderer : public Renderer<V, I>
    {
        using Base = Renderer<V, I>;
        std::vector<V> *vertices = 0;
        std::vector<V> temp_vertices;
        std::vector<I> temp_indices;
      public:
        ArrayRenderer() {}
        ArrayRenderer(std::vector<V> &vertices) : vertices(&vertices) {}

        bool HaveEnoughStorage(std::size_t, std::size_t) const override
        {
            return 1;
        }

        typename Base::Operation BeginOperation(std::size_t vertex_count) override
        {
            std::size_t old_size = vertices->size();
            vertices->resize(vertices->size() + vertex_count);
            return Base::Operation::Construct(this, vertices->data() + old_size, vertex_count);
        }

        void EndOperation(typename Base::Operation &op) override
        {
            op = {};
        }

        typename Base::IndexedOperation BeginOperationIndexed(std::size_t vertex_count, std::size_t index_count) override
        {
            if (vertex_count > 0 && vertices->size() + vertex_count - 1 > std::numeric_limits<I>::max())
                Program::Error("Index type is not large enough to support this many vertices.");
            temp_vertices.resize(vertex_count);
            temp_indices.resize(index_count);
            return Base::IndexedOperation::Construct(this, temp_vertices.data(), vertex_count, temp_indices.data(), index_count, 0);
        }

        void EndOperation(typename Base::IndexedOperation &op) override
        {
            std::size_t old_size = vertices->size();
            vertices->resize(vertices->size() + temp_indices.size());
            for (std::size_t i = 0; i < temp_indices.size(); i++)
                (*vertices)[old_size + i] = temp_vertices[temp_indices[i]];
            op = {};
        }

        void Cleanup() override
        {
            temp_vertices = {};
            temp_indices = {};
        }
    };
}
