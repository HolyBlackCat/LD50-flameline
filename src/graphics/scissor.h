#pragma once

#include <cglfl/cglfl.hpp>

#include "utils/mat.h"

namespace Graphics::Scissor
{
    inline void Enable()  {glEnable(GL_SCISSOR_TEST);}
    inline void Disable() {glDisable(GL_SCISSOR_TEST);}

    // Uses the same convention as `glScissor`, so the Y axis points up.
    inline void SetBounds(ivec2 pos, ivec2 size)
    {
        glScissor(pos.x, pos.y, size.x, size.y);
    }

    // Uses a more conventional coordinate system, where Y points downwards.
    inline void SetBounds_FlipY(ivec2 pos, ivec2 size, int framebuffer_height)
    {
        glScissor(pos.x, framebuffer_height - size.y - pos.y, size.x, size.y);
    }
}
