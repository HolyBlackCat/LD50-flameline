#include <iomanip>
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

GUI::Window win("Alpha", vec(800, 600), win.windowed, GUI::Window::Settings().Resizable());

int main(int, char**)
{
    while (1)
    {
        win.Tick();

        if (win.Resized())
        {
            std::cout << "Resized\n";
            glViewport(0, 0, win.Size().x, win.Size().y);
        }
        if (win.ExitRequested())
            return 0;

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
