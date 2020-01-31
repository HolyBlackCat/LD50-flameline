#pragma once

#include <utility>

#include <cglfl/cglfl.hpp>

#include "program/errors.h"
#include "macros/finally.h"

namespace Graphics
{
    class DummyVertexArray // Good for core profile
    {
        GLuint handle = 0;

      public:
        DummyVertexArray() {}

        DummyVertexArray(decltype(nullptr))
        {
            #ifdef GL_VERTEX_ARRAY_BINDING
            glGenVertexArrays(1, &handle);
            if (!handle)
                Program::Error("Unable to create a dummy vertex array object.");
            // Not needed because nothing can throw below this point:
            // FINALLY_ON_THROW( glDeleteVertexArrays(1, &handle); )
            glBindVertexArray(handle);
            #endif
        }

        DummyVertexArray(DummyVertexArray &&other) noexcept : handle(std::exchange(other.handle, {})) {}
        DummyVertexArray &operator=(DummyVertexArray other) noexcept
        {
            std::swap(handle, other.handle);
            return *this;
        }

        ~DummyVertexArray()
        {
            #ifdef GL_VERTEX_ARRAY_BINDING
            // The object is unbound automatically.
            if (handle)
                glDeleteVertexArrays(1, &handle); // Deleting 0 is a no-op, but GL could be unloaded at this point.
            #endif
        }
    };
}
