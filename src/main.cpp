#include <iostream>

#include "mat.h"

using namespace Math;

int main(int, char **)
{
    ivec3 v(1,2,3);
    std::cout << (v || ivec3(0,0,0));
}
