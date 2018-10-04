#pragma once

#include "utils/mat.h"

namespace States::Details::World
{
    class Camera
    {
        ivec2 pos = ivec2(0);
      public:
        void Tick();

        ivec2 Pos() const
        {
            return pos;
        }
        void SetPos(ivec2 new_pos)
        {
            pos = new_pos;
        }
    };
}
