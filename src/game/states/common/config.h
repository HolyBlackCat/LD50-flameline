#pragma once

#include "utils/config.h"

namespace Cfg
{
    ReflectStruct(ObjCommon,(
        (float)(fade_step)(=0.01),
    ))

    inline const auto &Common()
    {
        static Config<ObjCommon> ret("assets/config/common.refl");
        return ret;
    }
}

