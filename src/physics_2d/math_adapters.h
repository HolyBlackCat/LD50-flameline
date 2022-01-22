#pragma once

#include <box2d/b2_math.h>

#include "utils/mat.h"


// Interoperation between Box2D vectors and our vectors.

template <>
struct Math::Custom::Convert<b2Vec2, fvec2>
{
    fvec2 operator()(const b2Vec2 &v) const {return fvec2(v.x, v.y);}
};

template <>
struct Math::Custom::Convert<fvec2, b2Vec2>
{
    b2Vec2 operator()(const fvec2 &v) const {return b2Vec2(v.x, v.y);}
};

// Box2d provides more math primitives, but we didn't need them yet.
