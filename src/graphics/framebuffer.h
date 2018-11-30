#pragma once

#include <numeric>
#include <utility>
#include <vector>

#include <GLFL/glfl.h>

#include "graphics/texture.h"
#include "program/errors.h"
#include "utils/finally.h"
#include "utils/mat.h"

namespace Graphics
{
    class FrameBuffer
    {
        struct Data
        {
            GLuint handle = 0;
        };

        Data data;

        inline static GLuint binding = 0;

        struct Attachment
        {
            GLenum type;
            GLuint handle;

            Attachment(const TexObject &texture)
            {
                type = GL_TEXTURE_2D;
                handle = texture.Handle();
            }
            Attachment(const Texture &texture)
            {
                type = GL_TEXTURE_2D;
                handle = texture.Handle();
            }

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
        FrameBuffer(decltype(nullptr)) {}

        FrameBuffer()
        {
            glGenFramebuffers(1, &data.handle);
            if (!data.handle)
                Program::Error("Unable to create a framebuffer.");
            // Not needed because there is no code below this point:
            // FINALLY_ON_THROW( glDeleteFramebuffers(1, &data.handle); )
        }
        FrameBuffer(Attachment att) : FrameBuffer()
        {
            Attach(att);
        }
        FrameBuffer(const std::vector<Attachment> &att) : FrameBuffer()
        {
            Attach(att);
        }

        FrameBuffer(FrameBuffer &&other) noexcept : data(std::exchange(other.data, {})) {}
        FrameBuffer &operator=(FrameBuffer &&other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        ~FrameBuffer()
        {
            if (Bound())
                binding = 0; // Deleting a framebuffer unbinds it. We just need to adjust the saved binding.
            glDeleteFramebuffers(1, &data.handle); // Deleting 0 is a no-op.
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
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
            binding = handle;
        }

        void Bind() const
        {
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
            if (!*this)
                return std::move(*this);

            GLuint old_binding = binding;
            Bind();
            FINALLY( BindHandle(old_binding); )

            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, att.type, att.handle, 0);
            OnPC( glDrawBuffer(GL_COLOR_ATTACHMENT0); )

            return std::move(*this);
        }

        OnPC( FrameBuffer &&Attach(const std::vector<Attachment> &att) // Old non-depth attachments are discarded.
        {
            if (!*this)
                return std::move(*this);

            GLuint old_binding = binding;
            Bind();
            FINALLY( BindHandle(old_binding); )

            std::vector<GLenum> draw_buffers(att.size());
            std::iota(draw_buffers.begin(), draw_buffers.end(), GL_COLOR_ATTACHMENT0);

            for (size_t i = 0; i < att.size(); i++)
                glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, att[i].type, att[i].handle, 0);

            glDrawBuffers(draw_buffers.size(), draw_buffers.data());

            return std::move(*this);
        } )

        FrameBuffer &&AttachDepth(Attachment att) // Binds the framebuffer.
        {
            if (!*this)
                return std::move(*this);

            GLuint old_binding = binding;
            Bind();
            FINALLY( BindHandle(old_binding); )

            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, att.type, att.handle, 0);

            return std::move(*this);
        }
    };
}
