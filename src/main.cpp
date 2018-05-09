#include <iostream>

#include "mat.h"

int main(int, char **)
{
    std::cout << fvec3(1,2,3) /cross/ fvec3(2,3,4) << '\n';
    std::cout << fvec2(1,2) /cross/ fvec2(2,3);

}
