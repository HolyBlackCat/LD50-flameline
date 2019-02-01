#include <iostream>

#include "interface/window.h"
#include "graphics/complete.h"
#include "program/exit.h"
#include "utils/clock.h"
#include "utils/metronome.h"

int main()
{
    ivec2 window_size(800, 600);

    Interface::WindowSettings window_settings;
    window_settings.min_size = window_size / 2;

    Interface::Window window("Iota", window_size, Interface::windowed, window_settings);
    Metronome metronome(60);
    Clock::DeltaTimer delta_timer;

    while (1)
    {
        uint64_t delta = delta_timer();
        while (metronome.Tick(delta))
        {
            window.ProcessEvents();

            if (window.Resized())
            {
                Graphics::Viewport(window.Size());
                std::cout << "Resized.\n";
            }
            if (window.ExitRequested())
                Program::Exit();
        }

        Graphics::Clear();

        window.SwapBuffers();
    }
}
