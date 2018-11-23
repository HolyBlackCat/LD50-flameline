#pragma once

#include "program/errors.h"

namespace Program::impl
{
    [[maybe_unused]] static const int parachute_instance = []{
        SetErrorHandlers();
        return 0;
    }();
}
