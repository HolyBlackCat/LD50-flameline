#include <iostream>

#include "mat.h"

using namespace Math;

int main(int, char **)
{
    imat2 m(1,2,3,4);
    std::cout << m << "\n" << m.to_mat2x3();
}
