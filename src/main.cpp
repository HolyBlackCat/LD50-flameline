#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include "dynamic_storage.h"
#include "mat.h"
#include "reflection.h"
#include "strings.h"

#define main SDL_main

#include <cstdio>

struct Vec2
{
    float x = 0, y = 0;
    ReflectOptional(x, y)
};

struct A
{
    int foo = 0, bar = 0;
    float z = 0;
    int alpha = 0, beta = 0;
    Vec2 vector;

    Reflect(foo, bar, z)
    ReflectOptional(alpha, beta, vector)
};

//template <size_t ...I, typename F> void cexpr_for(std::index_sequence<I...>, F &&func)
//{
//    (func(std::integral_constant<size_t, I>{}) , ...);
//}

int main(int, char**)
{
//    cexpr_for(std::make_index_sequence<Refl::field_count<A>>{},[&](auto index)
//    {
//        if (index.value >= Refl::field_count_primary<A>)
//            std::cout << "(opt) ";
//        std::cout << Refl::field_name<A>(index.value) << " = " << Refl::field<index.value>(a) << '\n';
//    });

    A a;
    a.foo = 1;
    a.bar = 2;
    a.z = 3.4;
    a.alpha = -1;
    a.beta = -2;
    a.vector.x = 10;
    a.vector.y = 20;

    std::string str = Refl::to_string(a);
    std::cout << str << '\n';
    a = {};
    std::cout << Refl::to_string(a) << '\n';
    Refl::patch_from_string(a, str.c_str());
    std::cout << Refl::to_string(a) << '\n';
    a = {};
    std::cout << Refl::to_string(a) << '\n';
    Refl::patch_from_string(a, R"(
        {
            foo = 1, # a comment
            bar = 2,
            z   = 3.4000001,
            alpha = -1,
            beta  = -2,
            vector = {x=10,y=20},
        }
    )");
    std::cout << Refl::to_string(a) << '\n';

    return 0;
}
