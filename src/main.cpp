#include <iomanip>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <GLFL/glfl.h>

#include "input.h"
#include "messagebox.h"
#include "window.h"
#include "errors.h"
#include "exit.h"
#include "parachute.h"
#include "archive.h"
#include "dynamic_storage.h"
#include "mat.h"
#include "memory_file.h"
#include "reflection.h"
#include "strings.h"

#define main SDL_main

Program::Parachute error_parachute;
Interface::Window win("Alpha", vec(800, 600));

int main(int, char**)
{
    Interface::Button b;

    while (1)
    {
        win.ProcessEvents();

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

        if (b)
        {
            std::cout << " v" << b.pressed()
                      << " _" << b.down()
                      << " ^" << b.released()
                      << " ~" << b.up()
                      << " :" << b.repeated() << '\n';
        }
        else
        {
            if (b.AssignKey())
                std::cout << "Assigned " << b.Name() << "\n";
        }

        win.SwapBuffers();

        SDL_Delay(200);
    }

    return 0;
}
