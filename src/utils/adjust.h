#pragma once

#include "meta.h"

#define ADJUST(obj_, ...)   ( [&]{ auto _obj = obj_; MA_VA_FOR_EACH(IMPL_ADJUST, MA_NULL, , __VA_ARGS__) return _obj; }() )

// Clang doesn't like capturing lambdas at global scope, so here is a special version for global scope.
#define ADJUST_G(obj_, ...) ( [ ]{ auto _obj = obj_; MA_VA_FOR_EACH(IMPL_ADJUST, MA_NULL, , __VA_ARGS__) return _obj; }() )

#define IMPL_ADJUST(i, data, expr) _obj.expr;
