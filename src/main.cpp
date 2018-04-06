#include <iostream>

#include "mat.h"

using namespace Math;

template <typename T> struct S;

int main(int, char **)
{
    using T = propagate_qualifiers_t<const float &>;
    S<T> s;
}
