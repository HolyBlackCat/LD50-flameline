#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <GLFL/glfl.h>

#include "messagebox.h"
#include "window.h"
#include "errors.h"
#include "exit.h"
#include "dynamic_storage.h"
#include "mat.h"
#include "reflection.h"
#include "strings.h"

#define main SDL_main

int main(int, char**)
{
    GUI::Window win("Alpha", vec(800, 600), win.windowed);
    GUI::Window b;
    b = std::move(win);

    glfl::set_function_loader(SDL_GL_GetProcAddress);
    glfl::load_gl(3,3);

    while (1)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
            if (e.type == SDL_QUIT)
                Program::Exit();

        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
        glColor3f(0.9,0.1,0.6);
        glVertex2f(-0.5,0.5);
        glVertex2f(0.5,0.5);
        glVertex2f(0,-0.5);
        glEnd();

        win.Swap();
    }

    return 0;
}
