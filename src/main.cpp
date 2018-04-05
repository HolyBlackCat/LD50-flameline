#include <iostream>

#include "mat.h"

using namespace Math;

template <typename T> struct S;

int main(int, char **)
{
    std::cout << same_size_v<fmat3x2, fvec3>;
}
