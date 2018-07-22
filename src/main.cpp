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
#include "archive.h"
#include "dynamic_storage.h"
#include "mat.h"
#include "reflection.h"
#include "strings.h"

#define main SDL_main

Interface::Window win("Alpha", vec(800, 600));

int main(int, char**)
{
    Interface::Button b;

    std::string x = "Alpha, beta, gamma!";
    std::cout << "[" << x << "]\n";
    std::string y;
    y.resize(Archive::MaxCompressedSize((uint8_t*)&*x.begin(), (uint8_t*)&*x.end()));
    y.resize(Archive::Compress((uint8_t*)&*x.begin(), (uint8_t*)&*x.end(), (uint8_t*)&*y.begin(), (uint8_t*)&*y.end()) - (uint8_t*)&*y.begin());
    std::cout << "[" << y << "]\n";
    std::string z;
    z.resize(Archive::UncompressedSize((uint8_t*)&*y.begin(), (uint8_t*)&*y.end()));
    Archive::Uncompress((uint8_t*)&*y.begin(), (uint8_t*)&*y.end(), (uint8_t*)&*z.begin());
    std::cout << "[" << z << "]\n";

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
