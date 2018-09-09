#pragma once

#include <GLFL/glfl.h>

#include "utils/mat.h"

namespace Graphics
{
    inline void Clear(bool color = 1, bool depth = 0, bool stencil = 0)
    {
        glClear(color * GL_COLOR_BUFFER_BIT | depth * GL_DEPTH_BUFFER_BIT | stencil * GL_STENCIL_BUFFER_BIT);
    }

    inline void SetClearColor(fvec4 color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }
    inline void SetClearColor(fvec3 color)
    {
        SetClearColor(color.to_vec4(1));
    }
}
