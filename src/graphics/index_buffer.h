#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include <cglfl/cglfl.hpp>

#include "graphics/vertex_buffer.h"
#include "macros/finally.h"
#include "meta/misc.h"
#include "program/errors.h"

namespace Graphics
{
    class IndexBuffers
    {
        IndexBuffers() = delete;
        ~IndexBuffers() = delete;

        inline static GLuint binding = 0;

      public:
        static void Bind(GLuint handle)
        {
            if (binding == handle)
                return;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
            binding = handle;
        }

        static void ForgetBoundBuffer()
        {
            binding = 0;
        }

        static GLuint Binding()
        {
            return binding;
        }
    };

    template <typename T> inline constexpr bool is_valid_index_type_v = std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t> || std::is_same_v<T, std::uint32_t>;

    template <typename T>
    class IndexBuffer
    {
        // Note that some OpenGL versions don't support 32-bit indices. In this case using them will cause a runtime error.
        static_assert(is_valid_index_type_v<T>, "Invalid index buffer element type.");

        struct Data
        {
            GLuint handle = 0;
            int size = 0;
        };

        Data data;

      public:
        IndexBuffer() {}

        IndexBuffer(decltype(nullptr))
        {
            glGenBuffers(1, &data.handle);
            if (!data.handle)
                Program::Error("Unable to create an index buffer.");
            // Not needed because there is no code below this point:
            // FINALLY_ON_THROW( glDeleteBuffers(1, &handle); )
        }
        IndexBuffer(int count, const T *source = 0, Usage usage = static_draw) : IndexBuffer(nullptr) // Binds the buffer.
        {
            SetData(count, source, usage);
        }

        IndexBuffer(IndexBuffer &&other) noexcept : data(std::exchange(other.data, {})) {}
        IndexBuffer &operator=(IndexBuffer other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        ~IndexBuffer()
        {
            if (Bound())
                IndexBuffers::ForgetBoundBuffer(); // GL unbinds the buffer automatically.
            if (data.handle)
                glDeleteBuffers(1, &data.handle); // Deleting 0 is a no-op, but GL could be unloaded at this point.
        }

        explicit operator bool() const
        {
            return bool(data.handle);
        }

        GLuint Handle() const
        {
            return data.handle;
        }

        void Bind() const
        {
            DebugAssert("Attempt to use a null index buffer.", *this);
            if (!*this)
                return;
            IndexBuffers::Bind(data.handle);
        }
        static void Unbind()
        {
            IndexBuffers::Bind(0);
        }
        [[nodiscard]] bool Bound() const
        {
            return data.handle && data.handle == IndexBuffers::Binding();
        }

        int Size() const // This size is measured in elements, not bytes.
        {
            return data.size;
        }

        void SetData(int count, const T *source = 0, Usage usage = static_draw) // Binds the buffer.
        {
            DebugAssert("Attempt to use a null index buffer.", *this);
            if (!*this)
                return;
            Bind();
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(T), source, usage);
            data.size = count;
        }
        void SetDataPart(int elem_offset, int elem_count, const T *source) // Binds the buffer.
        {
            SetDataPartBytes(elem_offset * sizeof(T), elem_count * sizeof(T), (const uint8_t *)source);
        }
        void SetDataPartBytes(int offset, int bytes, const uint8_t *source) // Binds the buffer.
        {
            DebugAssert("Attempt to use a null index buffer.", *this);
            if (!*this)
                return;
            Bind();
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, bytes, source);
        }

        static GLenum IndexTypeEnum()
        {
            if constexpr (std::is_same_v<T, std::uint8_t>)
                return GL_UNSIGNED_BYTE;
            else if constexpr (std::is_same_v<T, std::uint16_t>)
                return GL_UNSIGNED_SHORT;
            else if constexpr (std::is_same_v<T, std::uint32_t>)
                return GL_UNSIGNED_INT;
            else // This shouldn't happen, since we already have a static assertion that validates the template paramer.
                static_assert(Meta::value<false, T>, "Invalid index buffer element type.");
        }

        void DrawFromBoundBuffer(DrawMode m, int offset, int count) const // Binds the buffer.
        {
            DebugAssert("Attempt to use a null index buffer.", *this);
            if (!*this)
                return;
            Bind();
            glDrawElements(m, count, IndexTypeEnum(), (void *)(uintptr_t)(offset * sizeof(T)));
        }
        void DrawFromBoundBuffer(DrawMode m, int count) const // Binds the buffer.
        {
            DrawFromBoundBuffer(m, 0, count);
        }
        void DrawFromBoundBuffer(DrawMode m) const // Binds the buffer.
        {
            DrawFromBoundBuffer(m, 0, Size());
        }

        template <typename V>
        void Draw(const VertexBuffer<V> &buffer, DrawMode m, int offset, int count) const // Binds this buffer (and well as the passed vertex buffer).
        {
            buffer.BindDraw();
            DrawFromBoundBuffer(m, offset, count);
        }

        template <typename V>
        void Draw(const VertexBuffer<V> &buffer, DrawMode m, int count) const // Binds this buffer (and well as the passed vertex buffer).
        {
            buffer.BindDraw();
            DrawFromBoundBuffer(m, count);
        }

        template <typename V>
        void Draw(const VertexBuffer<V> &buffer, DrawMode m) const // Binds this buffer (and well as the passed vertex buffer).
        {
            buffer.BindDraw();
            DrawFromBoundBuffer(m);
        }
    };
}
