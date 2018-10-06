#pragma once

#include "utils/mat.h"

namespace States {class World;}

namespace States::Details::World
{
    class Camera
    {
        ivec2 pixel_pos = ivec2(0);
        fvec2 pos = fvec2(0);
        fvec2 vel = fvec2(0);
        fvec2 target = fvec2(0);
        bool first_tick = 1;

      public:
        void Tick(States::World &world);

        ivec2 Pos() const
        {
            return pixel_pos;
        }
    };
}
