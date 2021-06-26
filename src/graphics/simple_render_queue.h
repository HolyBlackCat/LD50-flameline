#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <type_traits>
#include <vector>

#include "graphics/vertex_buffer.h"

namespace Graphics
{
    template <typename T, int N>
    class SimpleRenderQueue
    {
        static_assert(Graphics::VertexBuffer<T>::is_reflected, "The type must be reflected.");
        static_assert(N >= 1 && N <= 3, "N must be 1 (points), 2 (lines), or 3 (triangles).");

        std::size_t pos = 0, size = 0; // These are measured in primitives, not vertices.
        std::unique_ptr<T[]> storage;
        Graphics::VertexBuffer<T> buffer;

        template <typename ...P>
        void AddLow(const P &... p)
        {
            static_assert(sizeof...(P) == N);
            static_assert((std::is_same_v<P, T> && ...));
            if (pos >= size)
                Flush();
            int offset = 0;
            (std::copy_n(&p, 1, storage.get() + N * pos + offset++) , ...);
            pos++;
        }

      public:
        SimpleRenderQueue() {}

        // The size is measured in primitives, not vertices.
        SimpleRenderQueue(std::size_t size) : size(size), storage(std::make_unique<T[]>(size * N)), buffer(size * N, 0, Graphics::stream_draw) {}

        [[nodiscard]] explicit operator bool()
        {
            return bool(storage);
        }

        // Returns true if the next operation would flush.
        [[nodiscard]] bool Full()
        {
            return pos == size;
        }

        // How many primitives are currently in the queue.
        [[nodiscard]] std::size_t Pos() const
        {
            return pos;
        }

        // The max number of primitives the queue can hold.
        [[nodiscard]] std::size_t Size() const
        {
            return size;
        }

        void Flush()
        {
            if (pos <= 0)
                return;
            buffer.SetDataPart(0, pos * N, storage.get());
            buffer.Draw(std::array{points, lines, triangles}[N-1], pos * N);
            pos = 0;
        }

        void Add(const T &a) requires (N == 1)
        {
            AddLow(a);
        }
        void Add(const T &a, const T &b) requires (N == 2)
        {
            AddLow(a, b);
        }
        void Add(const T &a, const T &b, const T &c) requires (N == 3)
        {
            AddLow(a, b, c);
        }
        void Add(const T &a, const T &b, const T &c, const T &d) requires (N == 3/*sic*/)
        {
            AddLow(a, b, d);
            AddLow(d, b, c);
        }
    };
}
