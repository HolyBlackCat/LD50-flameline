#pragma once

#include <cstddef>

#include <SDL2/SDL_timer.h>

namespace Clock
{
    inline uint64_t Time()
    {
        return SDL_GetPerformanceCounter();
    }

    inline uint64_t TicksPerSecond()
    {
        static uint64_t ret = SDL_GetPerformanceFrequency();
        return ret;
    }

    inline uint64_t SecondsToTicks(double secs)
    {
        return secs * TicksPerSecond();
    }
    inline double TicksToSeconds(uint64_t ticks)
    {
        return ticks / double(TicksPerSecond());
    }
}
