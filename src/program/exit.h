#pragma once

#include <cstdlib>

namespace Program
{
    [[noreturn]] inline void Exit(int code = 0)
    {
        std::exit(code);
    }
}
