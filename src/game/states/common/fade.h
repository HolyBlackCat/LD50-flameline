#pragma once

#include "utils/mat.h"

namespace States::Details::Common
{
    class Fade
    {
        fvec3 color;
        float state = 1;
      public:
        Fade(fvec3 color) : color(color) {}
        void Tick();
        void Render() const;
    };
}
