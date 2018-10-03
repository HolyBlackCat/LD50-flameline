#pragma once

#include "utils/config.h"

namespace Cfg
{
    ReflectStruct(Menu,(
        (ivec2)(button_offset)(=ivec2(0,50)),
        (int)(button_y_step)(=32),
        (float)(button_time_step)(=0.02),
        (fvec3)(button_color_shadow)(=fvec3(0)),
        (fvec3)(button_color_main)(=fvec3(1)),
        (fvec3)(button_color_active)(=fvec3(1, 0.75, 0.5)),
    ))

    inline static const auto menu = Config<Menu>("assets/config/menu.refl");
}
