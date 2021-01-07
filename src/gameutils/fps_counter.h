#pragma once

#include "interface/window.h"

namespace GameUtils
{
    class FpsCounter
    {
        int last_second = -1;
        decltype(Interface::Window::Get().Ticks()) last_ticks = 0;
        decltype(Interface::Window::Get().Frames()) last_frames = 0;
        int tps = 0;
        int fps = 0;

      public:
        constexpr FpsCounter() {}

        // Prefer to call this once per frame.
        // Returns true if the values were updated, false otherwise.
        bool Update()
        {
            int this_second = SDL_GetTicks() / 1000;
            if (this_second == last_second)
                return false;
            last_second = this_second;
            Interface::Window window = Interface::Window::Get();
            tps = window.Ticks() - std::exchange(last_ticks, window.Ticks());
            fps = window.Frames() - std::exchange(last_frames, window.Frames());
            return true;
        }

        [[nodiscard]] int Tps() const
        {
            return tps;
        }
        [[nodiscard]] int Fps() const
        {
            return fps;
        }
    };
}
