#include <iostream>

#include "mat.h"

using namespace Math;

int main(int, char **)
{
    ivec3 v(1,2,3);
    std::cout << v.div_y(10.0);
}
