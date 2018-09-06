#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <type_traits>
#include <vector>

#include "vertex_buffer.h"

namespace Graphics
{
    template <typename T, int N> class RenderQueue
    {
        static_assert(Graphics::VertexBuffer<T>::is_reflected, "The type must be reflected.");
        static_assert(N >= 1 && N <= 3, "N must be 1 (points), 2 (lines), or 3 (triangles).");

        int pos = 0, size = 0; // These are measured in primitives, not vertices.
        std::unique_ptr<T[]> storage;
        Graphics::VertexBuffer<T> buffer = nullptr;

        template <typename ...P> void AddLow(const P &... p)
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
        RenderQueue(decltype(nullptr)) {}
        RenderQueue(int size) : size(size), storage(std::make_unique<T[]>(size * N)), buffer(size * N, 0, Graphics::stream_draw) {}

        explicit operator bool()
        {
            return bool(storage);
        }

        int Size() const
        {
            return size;
        }

        void Flush()
        {
            if (pos <= 0)
                return;
            buffer.SetDataPart(0, size * N, storage.get());
            buffer.Draw(std::array{points, lines, triangles}[N-1]);
            pos = 0;
        }

        void Add(const T &a)
        {
            static_assert(N == 1, "Incorrect parameter count.");
            AddLow(a);
        }
        void Add(const T &a, const T &b)
        {
            static_assert(N == 2, "Incorrect parameter count.");
            AddLow(a, b);
        }
        void Add(const T &a, const T &b, const T &c)
        {
            static_assert(N == 3, "Incorrect parameter count.");
            AddLow(a, b, c);
        }
        void Add(const T &a, const T &b, const T &c, const T &d)
        {
            static_assert(N == 3/*sic*/, "Incorrect parameter count.");
            AddLow(a, b, d);
            AddLow(d, b, c);
        }
    };
}
