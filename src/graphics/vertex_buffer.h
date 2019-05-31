#pragma once

#include <type_traits>
#include <utility>

#include <GLFL/glfl.h>

#include "program/errors.h"
#include "reflection/complete.h"
#include "utils/finally.h"
#include "utils/mat.h"
#include "utils/meta.h"

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
        // `attributes` is effectively unused. We need it to compute attribute offsets.
        template <typename T> static void BindDraw(GLuint handle, const T &attributes)
        {
            if (handle == 0) // Null handle is a special case.
            {
                // Note that we disable attributes unconditionally. We don't want to insert `if (binding_draw != 0)` here.
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
                using refl_t = Refl::Interface<const T>;
                refl_t refl(attributes);

                int attrib_index = 0;

                refl_t::for_each_field([&](auto index)
                {
                    constexpr int i = index.value;
                    using field_type = typename refl_t::template field_type<i>;
                    using base_type = Math::vec_base_t<field_type>;

                    GLint type_enum;

                    if constexpr (std::is_same_v<base_type, char>)
                        type_enum = (std::is_signed_v<char> ? GL_BYTE : GL_UNSIGNED_BYTE);
                    else if constexpr (std::is_same_v<base_type, signed char>)
                        type_enum = GL_BYTE;
                    else if constexpr (std::is_same_v<base_type, unsigned char>)
                        type_enum = GL_UNSIGNED_BYTE;
                    else if constexpr (std::is_same_v<base_type, short>)
                        type_enum = GL_SHORT;
                    else if constexpr (std::is_same_v<base_type, unsigned short>)
                        type_enum = GL_UNSIGNED_SHORT;
                    #ifdef GL_INT
                    else if constexpr (std::is_same_v<base_type, int>)
                        type_enum = GL_INT;
                    else if constexpr (std::is_same_v<base_type, unsigned int>)
                        type_enum = GL_UNSIGNED_INT;
                    #endif
                    else if constexpr (std::is_same_v<base_type, float>)
                        type_enum = GL_FLOAT;
                    else
                        static_assert(Meta::value<false, T, decltype(index)>, "Attributes of this type are not supported.");

                    uintptr_t offset = reinterpret_cast<const char *>(&refl.template field_value<i>()) - reinterpret_cast<const char *>(&attributes);
                    glVertexAttribPointer(attrib_index++, Math::vec_size_v<field_type>, type_enum, 0, sizeof(T), (void *)offset);
                });
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

        VertexBuffer() {}

        VertexBuffer(decltype(nullptr))
        {
            glGenBuffers(1, &data.handle);
            if (!data.handle)
                Program::Error("Unable to create a vertex buffer.");
            // Not needed because there is no code below this point:
            // FINALLY_ON_THROW( glDeleteBuffers(1, &handle); )
        }
        VertexBuffer(int count, const T *source = 0, Usage usage = static_draw) : VertexBuffer(nullptr) // Binds storage.
        {
            SetData(count, source, usage);
        }

        VertexBuffer(VertexBuffer &&other) noexcept : data(std::exchange(other.data, {})) {}
        VertexBuffer &operator=(VertexBuffer &&other) noexcept
        {
            data = std::exchange(other.data, {});
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

        void BindDraw(const T &attributes = {}) const // If element type is not reflected, disables all attributes. `attributes` is effectively unused. We need it to compute attribute offsets.
        {
            if (!*this)
                return;
            Buffers::BindDraw(data.handle, attributes);
        }
        static void UnbindDraw() // Disables all attributes. If any buffer is currently bound, this results in stripping draw binding from it.
        {
            Buffers::BindDraw(0, nullptr);
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
}
