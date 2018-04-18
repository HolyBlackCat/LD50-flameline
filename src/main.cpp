#include <iostream>

#include "mat.h"

using namespace Math;

int main(int, char **)
{
    std::cout << fmat2(0,0,1,1).mul(fvec2(2,3));
}
