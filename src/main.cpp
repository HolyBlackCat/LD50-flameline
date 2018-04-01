#include <iostream>

#include "mat.h"

int main(int, char **)
{
    Math::fvec4 m(1.2,2.2,3.2,4.2);
    for (int x = 0; x < 4; x++)
        std::cout << m[x] << ' ';
    std::cout << '\n';
    for (int x = 0; x < 4; x++)
        std::cout << m.to<int>()[x] << ' ';
}
