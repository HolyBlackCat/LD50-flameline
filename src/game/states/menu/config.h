#pragma once

#include "utils/config.h"

namespace Cfg
{
    ReflectStruct(ObjMenu,(
        (ivec2)(button_offset)(=ivec2(0,50)),
        (int)(button_y_step)(=32),
        (int)(button_y_size)(=32),
        (float)(button_time_step)(=0.02),
        (fvec3)(button_color_shadow)(=fvec3(0)),
        (fvec3)(button_color_main)(=fvec3(1)),
        (fvec3)(button_color_active)(=fvec3(1, 0.75, 0.5)),
    ))

    inline const auto &Menu()
    {
        static Config<ObjMenu> ret("assets/config/menu.refl");
        return ret;
    }
}
