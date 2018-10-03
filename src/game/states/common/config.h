#pragma once

#include "utils/config.h"

namespace Cfg
{
    ReflectStruct(Common,(
        (float)(fade_step)(=0.01),
    ))

    inline static const auto common = Config<Common>("assets/config/common.refl");
}

