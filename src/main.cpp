#include <iostream>
#include <unordered_map>

#include "mat.h"

using namespace Math;

template <typename T> struct S;


int main(int, char **)
{
    std::unordered_map<fvec3, int> m;
    m[{1,2,3}] = 4;
    std::cout << m[{1,2,3}];
}
