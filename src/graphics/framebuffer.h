#pragma once

#include <numeric>
#include <utility>
#include <vector>

#include <cglfl/cglfl.hpp>

#include "graphics/texture.h"
#include "program/errors.h"
#include "macros/finally.h"
#include "utils/mat.h"

namespace Graphics
{
    #if defined(GL_FRAMEBUFFER_BINDING) || defined(GL_DRAW_FRAMEBUFFER_BINDING)
    #  define IMP_RE_HAVE_FRAMEBUFFERS
    #endif

    #ifdef IMP_RE_HAVE_FRAMEBUFFERS
    class FrameBuffer
    {
        struct Data
        {
            GLuint handle = 0;
        };

        Data data;

        inline static GLuint binding = 0;

        static constexpr GLenum binding_point =
        #ifdef GL_DRAW_FRAMEBUFFER
            GL_DRAW_FRAMEBUFFER;
        #else
            GL_FRAMEBUFFER;
        #endif

        struct Attachment
        {
            GLenum type;
            GLuint handle;

            Attachment(const TexObject &texture)
            {
                DebugAssert("Attempt to use a null texture.", texture);
                type = GL_TEXTURE_2D;
                handle = texture.Handle();
            }
            Attachment(const Texture &texture) : Attachment(texture.Object()) {}

            /*
            Some day we'll have renderbuffers.

            Attachment(const RenderBuffer &renderbuffer)
            {
                type = GL_RENDERBUFFER;
                handle = renderbuffer.Handle();
            }
            */
        };

      public:
        FrameBuffer() {}

        FrameBuffer(decltype(nullptr))
        {
            glGenFramebuffers(1, &data.handle);
            if (!data.handle)
                Program::Error("Unable to create a framebuffer.");
            // Not needed because there is no code below this point:
            // FINALLY_ON_THROW( glDeleteFramebuffers(1, &data.handle); )
        }
        FrameBuffer(Attachment att) : FrameBuffer(nullptr)
        {
            Attach(att);
        }
        #ifdef glDrawBuffers
        FrameBuffer(const std::vector<Attachment> &att) : FrameBuffer(nullptr)
        {
            Attach(att);
        }
        #endif

        FrameBuffer(FrameBuffer &&other) noexcept : data(std::exchange(other.data, {})) {}
        FrameBuffer &operator=(FrameBuffer other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        ~FrameBuffer()
        {
            if (Bound())
                binding = 0; // Deleting a framebuffer unbinds it. We just need to adjust the saved binding.
            if (data.handle)
                glDeleteFramebuffers(1, &data.handle); // Deleting 0 is a no-op, but GL could be unloaded at this point.
        }

        explicit operator bool() const
        {
            return bool(data.handle);
        }

        GLuint Handle() const
        {
            return data.handle;
        }

        static void BindHandle(GLuint handle)
        {
            if (binding == handle)
                return;
            glBindFramebuffer(binding_point, handle);
            binding = handle;
        }

        void Bind() const
        {
            DebugAssert("Attempt to use a null framebuffer.", *this);
            if (!*this)
                return;
            BindHandle(data.handle);
        }
        static void BindDefault()
        {
            BindHandle(0);
        }
        [[nodiscard]] bool Bound() const
        {
            return data.handle && binding == data.handle;
        }

        FrameBuffer &&Attach(Attachment att) // Old non-depth attachments are discarded.
        {
            DebugAssert("Attempt to use a null framebuffer.", *this);
            if (!*this)
                return std::move(*this);

            GLuint old_binding = binding;
            Bind();
            FINALLY( BindHandle(old_binding); )

            glFramebufferTexture2D(binding_point, GL_COLOR_ATTACHMENT0, att.type, att.handle, 0);
            #ifdef glDrawBuffer
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            #endif

            return std::move(*this);
        }

        #ifdef glDrawBuffers
        FrameBuffer &&Attach(const std::vector<Attachment> &att) // Old non-depth attachments are discarded.
        {
            DebugAssert("Attempt to use a null framebuffer.", *this);
            if (!*this)
                return std::move(*this);

            GLuint old_binding = binding;
            Bind();
            FINALLY( BindHandle(old_binding); )

            for (size_t i = 0; i < att.size(); i++)
                glFramebufferTexture2D(binding_point, GL_COLOR_ATTACHMENT0 + i, att[i].type, att[i].handle, 0);

            std::vector<GLenum> draw_buffers(att.size());
            std::iota(draw_buffers.begin(), draw_buffers.end(), GL_COLOR_ATTACHMENT0);
            glDrawBuffers(draw_buffers.size(), draw_buffers.data());

            return std::move(*this);
        }
        #endif

        FrameBuffer &&AttachDepth(Attachment att)
        {
            DebugAssert("Attempt to use a null framebuffer.", *this);
            if (!*this)
                return std::move(*this);

            GLuint old_binding = binding;
            Bind();
            FINALLY( BindHandle(old_binding); )

            glFramebufferTexture2D(binding_point, GL_DEPTH_ATTACHMENT, att.type, att.handle, 0);

            return std::move(*this);
        }

        FrameBuffer &&CheckStatus() // Throws if the framebuffer is incomplete.
        {
            DebugAssert("Attempt to use a null framebuffer.", *this);
            if (!*this)
                return std::move(*this);

            GLuint old_binding = binding;
            Bind();
            FINALLY( BindHandle(old_binding); )

            switch (glCheckFramebufferStatus(binding_point))
            {
              case GL_FRAMEBUFFER_COMPLETE:
                return std::move(*this);
              case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                Program::Error("Bad framebuffer status: Incomplete (attachment).");
              case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                Program::Error("Bad framebuffer status: Incomplete (missing attachment).");
              case GL_FRAMEBUFFER_UNSUPPORTED:
                Program::Error("Bad framebuffer status: Incomplete (unsupported).");
                #ifdef GL_FRAMEBUFFER_UNDEFINED
              case GL_FRAMEBUFFER_UNDEFINED:
                Program::Error("Bad framebuffer status: Undefined.");
                #endif
                #ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
              case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                Program::Error("Bad framebuffer status: Incomplete (draw buffer).");
                #endif
                #ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
              case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                Program::Error("Bad framebuffer status: Incomplete (read buffer).");
                #endif
                #ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
              case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                Program::Error("Bad framebuffer status: Incomplete (multisample).");
                #endif
                #ifdef GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
              case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                Program::Error("Bad framebuffer status: Incomplete (layer targets).");
                #endif
              default:
                Program::Error("Bad framebuffer status: Unknown.");
            }
        }
    };
    #endif
}
