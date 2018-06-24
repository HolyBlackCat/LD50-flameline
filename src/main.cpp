#include <iostream>
#include <unordered_map>

#include "mat.h"

using namespace Math;

template <typename T> struct S;


int main(int, char **)
{
    const int a = 10;
    ivec3 b(1,2,3);
    Math::apply_elementwise([](int &x, int y){x += y;}, b, a);
    std::cout << b;
}
