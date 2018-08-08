#include <iomanip>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>
#include <GLFL/glfl.h>

#include "graphics/image.h"
#include "interface/input.h"
#include "interface/messagebox.h"
#include "interface/window.h"
#include "program/errors.h"
#include "program/exit.h"
#include "program/parachute.h"
#include "utils/archive.h"
#include "utils/dynamic_storage.h"
#include "utils/finally.h"
#include "utils/macro.h"
#include "utils/mat.h"
#include "utils/memory_file.h"
#include "reflection/interface.h"
#include "reflection/containers_std.h"
#include "reflection/primitives_arithmetic.h"
#include "reflection/structures_macro.h"
#include "utils/strings.h"

#define main SDL_main

#include <unordered_set>

Program::Parachute error_parachute;
Interface::Window win("Alpha", vec(800, 600));
Graphics::Image img("test.png");

struct A
{
    Reflect(A)
    (
        (int)(x,y),
        (float)(z)(=42),
        (optional)(float)(w)(),
    )
};

struct B
{
    Reflect(B)
    (
        (A)(a),
        (std::vector<float>)(alpha)(={1.1,2.2,3.3}),
    )
};

int main(int, char**)
{
    B obj;
    obj.a.x = 1;
    obj.a.y = 2;
    obj.a.z = 3.3;
    obj.a.w = 4.4;
    auto refl = Refl::Interface(obj);
    std::cout << refl.to_string() << '\n';


    //std::cout << .field_name(2) << '\n';

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
