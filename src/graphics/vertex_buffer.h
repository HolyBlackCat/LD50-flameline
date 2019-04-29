#pragma once

#include <type_traits>
#include <utility>

#include <GLFL/glfl.h>

#include "program/errors.h"
#include "reflection/complete.h"
#include "utils/finally.h"
#include "utils/mat.h"

namespace Graphics
{
    enum DrawMode
    {
        points         = GL_POINTS,
        lines          = GL_LINES,
        line_strip     = GL_LINE_STRIP,
        line_loop      = GL_LINE_LOOP,
        triangles      = GL_TRIANGLES,
        triangle_strip = GL_TRIANGLE_STRIP,
        triangle_fan   = GL_TRIANGLE_FAN,
    };

    enum Usage
    {
        static_draw  = GL_STATIC_DRAW,
        dynamic_draw = GL_DYNAMIC_DRAW,
        stream_draw  = GL_STREAM_DRAW,
    };

    class Buffers
    {
        Buffers() = delete;
        ~Buffers() = delete;

        // Only one buffer can be bound at a time. Optionally it can be draw-bound at the same time.
        inline static GLuint binding = 0;
        inline static GLuint binding_draw = 0;

        inline static int active_attrib_count = 0;

        static void SetActiveAttribCount(int count)
        {
            if (count == active_attrib_count)
                return;
            if (active_attrib_count < count)
                do glEnableVertexAttribArray(active_attrib_count++); while (active_attrib_count < count);
            else
                do glDisableVertexAttribArray(--active_attrib_count); while (active_attrib_count > count);
        }

      public:
        // Simply binds the VBO if it's not already bound.
        static void BindStorage(GLuint handle)
        {
            if (binding == handle)
                return;
            glBindBuffer(GL_ARRAY_BUFFER, handle);
            binding = handle;
            binding_draw = 0;
        }

        // First, binds storage for the same handle if necessary. Then sets attribute pointers if T is reflected, otherwise disables all attributes.
        // BindDraw(0) is a special case. It disables all attributes, and thus strips draw binding from currently bound buffer (if any).
        template <typename T> static void BindDraw(GLuint handle)
        {
            if (handle == 0) // Null handle is a special case.
            {
                // Note that we disable attributes unconditionally. We can't insert `if (bind_draw != 0)` here.
                SetActiveAttribCount(0);
                binding_draw = 0;
                return;
            }

            if (binding_draw == handle)
                return;
            BindStorage(handle);

            constexpr bool is_reflected = Refl::is_reflected<T>;
            int field_count;
            if constexpr (is_reflected)
                field_count = Refl::Interface<T>::field_count();
            else
                field_count = 0;

            SetActiveAttribCount(field_count);

            if constexpr (is_reflected)
            {
                using refl = Refl::Interface<T>;

                int offset = 0, attrib = 0;

                refl::for_each_field([&](auto index)
                {
                    constexpr int i = index.value;
                    using field_type = typename refl::template field_type<i>;
                    static_assert(std::is_same_v<Math::vec_base_t<field_type>, float>, "Non-float attributes are not supported.");
                    glVertexAttribPointer(attrib++, Math::vec_size_v<field_type>, GL_FLOAT, 0, sizeof(T), (void *)(uintptr_t)offset);
                    offset += sizeof(field_type);
                });

                if (offset != int(sizeof(T)))
                    Program::Error("Unexpected padding in attribute structure.");
            }

            binding_draw = handle;
        }

        static void ForgetBoundBuffer() // Assume no buffer is bound, but don't actually unbind anything. Useful if currently bound buffer is going to be deleted immediately.
        {
            binding = 0;
            binding_draw = 0;
        }

        static GLuint StorageBinding()
        {
            return binding;
        }
        static GLuint DrawBinding()
        {
            return binding_draw;
        }
    };

    template <typename T> class VertexBuffer
    {
        static_assert(!std::is_void_v<T>, "Element type can't be void. Use uint8_t instead.");

        struct Data
        {
            GLuint handle = 0;
            int size = 0;
        };
        Data data;

      public:
        static constexpr bool is_reflected = Refl::is_reflected<T>;

        VertexBuffer(decltype(nullptr)) {}

        VertexBuffer()
        {
            glGenBuffers(1, &data.handle);
            if (!data.handle)
                Program::Error("Unable to create a vertex buffer.");
            // Not needed because there is no code below this point:
            // FINALLY_ON_THROW( glDeleteBuffers(1, &handle); )
        }
        VertexBuffer(int count, const T *source = 0, Usage usage = static_draw) : VertexBuffer() // Binds storage.
        {
            SetData(count, source, usage);
        }

        VertexBuffer(VertexBuffer &&other) noexcept : data(std::exchange(other.data, {})) {}
        VertexBuffer &operator=(VertexBuffer other) noexcept // Note the pass by value to utilize copy&swap idiom.
        {
            std::swap(data, other.data);
            return *this;
        }

        ~VertexBuffer()
        {
            if (StorageBound())
                Buffers::ForgetBoundBuffer(); // GL unbinds the buffer automatically.
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

        void BindStorage() const
        {
            if (!*this)
                return;
            Buffers::BindStorage(data.handle);
        }
        static void UnbindStorage() // Removes draw binding as well. Doesn't disable any attributes for performance reasons.
        {
            Buffers::BindStorage(0);
        }
        [[nodiscard]] bool StorageBound() const
        {
            return data.handle && data.handle == Buffers::StorageBinding();
        }

        void BindDraw() const // If element type is not reflected, disables all attributes.
        {
            if (!*this)
                return;
            Buffers::BindDraw<T>(data.handle);
        }
        static void UnbindDraw() // Disables all attributes. If any buffer is currently bound, this results in stripping draw binding from it.
        {
            Buffers::BindDraw<void>(0);
        }
        [[nodiscard]] bool DrawBound() const
        {
            return data.handle && data.handle == Buffers::DrawBinding();
        }

        int Size() const // This size is measured in elements, not bytes.
        {
            return data.size;
        }

        void SetData(int count, const T *source = 0, Usage usage = static_draw) // Binds storage.
        {
            if (!*this)
                return;
            BindStorage();
            // It seems that glBufferData doesn't invalidate attribute pointers, so we don't need to get rid of draw binding here.
            glBufferData(GL_ARRAY_BUFFER, count * sizeof(T), source, usage);
            data.size = count;
        }
        void SetDataPart(int elem_offset, int elem_count, const T *source) // Binds storage.
        {
            SetDataPartBytes(elem_offset * sizeof(T), elem_count * sizeof(T), (const uint8_t *)source);
        }
        void SetDataPartBytes(int offset, int bytes, const uint8_t *source) // Binds storage.
        {
            if (!*this)
                return;
            BindStorage();
            glBufferSubData(GL_ARRAY_BUFFER, offset, bytes, source);
        }

        void Draw(DrawMode p, int from, int count) // Binds for drawing.
        {
            static_assert(is_reflected, "Element type of this buffer is not reflected, unable to draw.");
            if (!*this)
                return;
            BindDraw();
            glDrawArrays(p, from, count);
        }
        void Draw(DrawMode p, int count) // Binds for drawing.
        {
            Draw(p, 0, count);
        }
        void Draw(DrawMode p) // Binds for drawing.
        {
            Draw(p, 0, Size());
        }
    };

    struct DummyVertexArray // Good for core profile
    {
        DummyVertexArray()
        {
            GLuint vao;
            glGenVertexArrays(1, &vao);
            if (!vao)
                Program::Error("Unable to create the dummy vertex array object.");
            glBindVertexArray(vao);
        }
        DummyVertexArray(const DummyVertexArray &) = delete;
        DummyVertexArray &operator=(const DummyVertexArray &) = delete;
    };
}
