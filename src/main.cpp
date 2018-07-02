#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include "dynamic_storage.h"
#include "mat.h"
#include "reflection.h"
#include "strings.h"

#define main SDL_main

#include <cstdio>

struct A
{
    int foo = 1, bar = 2;
    float z = 3.4;

    Reflect(foo,bar, z)
};

int main(int, char**)
{
    A a;
    std::cout << Refl::field_name<A>(0) << " = " << Refl::field<0>(a) << '\n';
    std::cout << Refl::field_name<A>(1) << " = " << Refl::field<1>(a) << '\n';
    std::cout << Refl::field_name<A>(2) << " = " << Refl::field<2>(a) << '\n';
    return 0;
}
