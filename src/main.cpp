#include <iostream>

#include "mat.h"

using namespace Math;

int main(int, char **)
{
    ivec3 v(1,2,3);
    std::cout << v.len_sqr() << ' ' << v.len() << ' ' << v.norm() << ' ' << v.norm().len();
}
