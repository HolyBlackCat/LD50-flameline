#ifndef GRAPHICS_VIEWPORT_H_INCLUDED
#define GRAPHICS_VIEWPORT_H_INCLUDED

#include <GLFL/glfl.h>

#include "utils/mat.h"

namespace Graphics
{
    inline void Viewport(ivec2 pos, ivec2 size)
    {
        glViewport(pos.x, pos.y, size.x, size.y);
    }
    inline void Viewport(ivec2 size)
    {
        Viewport(ivec2(0), size);
    }
}

#endif
