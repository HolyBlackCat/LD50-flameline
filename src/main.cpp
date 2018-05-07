#include <iostream>

#include "mat.h"

using namespace Math;

int main(int, char **)
{
    std::cout << fmat4::perspective(1.2, 0.75, 1, 2) << '\n';
}
