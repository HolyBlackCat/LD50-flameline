#pragma once

#include "input/complete.h"

struct Controls
{
    Input::Button
        up      = Input::up,
        down    = Input::down,
        left    = Input::left,
        right   = Input::right,
        jump_ok = Input::f;

    Input::Mouse mouse;
};

inline Controls controls;
