#pragma once

#include <cmath>
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

    inline void WaitSeconds(double secs) // Waits the specified amount of seconds, rounded down to milliseconds.
    {
        SDL_Delay(int(secs * 1000));
    }

    class DeltaTimer
    {
        uint64_t time = 0;
      public:
        DeltaTimer() : time(Time()) {} // Don't call before SDL initialization.

        uint64_t operator()()
        {
            uint64_t new_time = Time(), delta = new_time - time;
            time = new_time;
            return delta;
        }

        uint64_t LastTimePoint() const
        {
            return time;
        }
    };
}
