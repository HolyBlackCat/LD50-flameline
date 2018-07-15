#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include "messagebox.h"
#include "window.h"
#include "errors.h"
#include "exit.h"
#include "dynamic_storage.h"
#include "handle.h"
#include "mat.h"
#include "reflection.h"
#include "strings.h"

#define main SDL_main

int main(int, char**)
{
    GUI::Window win("Alpha", vec(800, 600));

    while (1)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
            if (e.type == SDL_QUIT)
                Program::Exit();
    }

    return 0;
}
