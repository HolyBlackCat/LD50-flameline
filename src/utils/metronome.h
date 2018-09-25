#pragma once

#include <cstddef>

#include "utils/clock.h"

class Metronome
{
    uint64_t tick_len;
    int max_ticks;

    uint64_t accumulator;
    bool new_frame;
    bool lag;

    float comp_th = 0, comp_amount = 0;
    int comp_dir = 0; // Internal. 1 means forward, -1 means backwards, 0 means whatever is better.

  public:
    uint64_t ticks = 0;

    Metronome(decltype(nullptr)) {} // For any other constructor to work, the clock has to be initialized first.

    Metronome() : Metronome(60) {}
    Metronome(double freq, int max_ticks_per_frame = 8, float compensation_threshold = 0.01, float compensation_amount = 0.5)
    {
        SetFrequency(freq);
        SetMaxTicksPerFrame(max_ticks_per_frame);
        SetCompensation(compensation_threshold, compensation_amount);
        Reset();
    }

    void SetFrequency(double freq)
    {
        tick_len = Clock::TicksPerSecond() / freq;
    }
    void SetMaxTicksPerFrame(int n) // Set to 0 to disable the limit.
    {
        max_ticks = n;
    }

    // Threshold should be positive and small, at least less than 1.
    // Amount should be at least two times larger (by a some margin) than threshold, otherwise it will break. 0.5 should give best results, but don't make it much larger.
    // When the abs('frame len' - 'tick len') / 'tick_len' < 'threshold', the compensator kicks in and adds or subtracts 'amount' * 'tick len' from the time.
    void SetCompensation(float threshold, float amount)
    {
        comp_th = threshold;
        comp_amount = amount;
    }
    void Reset()
    {
        accumulator = 0;
        new_frame = 1;
        lag = 0;
        comp_dir = 0;
        ticks = 0;
    }

    bool Lag() // Flag resets after this function is called. The flag is set to 1 if the amount of ticks per last frame is at maximum value.
    {
        if (lag)
        {
            lag = 0;
            return 1;
        }
        return 0;
    }

    double Frequency() const
    {
        return Clock::TicksPerSecond() / double(tick_len);
    }

    uint64_t ClockTicksPerTick() const
    {
        return tick_len;
    }

    int MaxTicksPerFrame() const
    {
        return max_ticks;
    }

    bool Tick(uint64_t delta)
    {
        if (new_frame)
            accumulator += delta;

        if (abs(int64_t(accumulator - tick_len)) < tick_len * comp_th)
        {
            int dir;
            if (comp_dir)
                dir = -comp_dir;
            else
                dir = (accumulator < tick_len ? -1 : 1);

            comp_dir += dir;
            accumulator += tick_len * comp_amount * dir;
        }

        if (accumulator >= tick_len)
        {
            if (max_ticks && accumulator > tick_len * max_ticks)
            {
                accumulator = tick_len * max_ticks;
                lag = 1;
            }
            accumulator -= tick_len;
            new_frame = 0;
            ticks++;
            return 1;
        }
        else
        {
            new_frame = 1;
            return 0;
        }
    }

    double Time() const
    {
        return accumulator / double(tick_len);
    }
};
