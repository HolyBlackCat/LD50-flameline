#pragma once

#include "utils/macro.h"

namespace Macro
{
    template <typename F> struct WithExpr
    {
        F lambda;

        constexpr WithExpr(F &&lambda) : lambda(std::move(lambda)) {}

        template <typename T> friend auto operator->*(T object, WithExpr &&expr)
        {
            expr.lambda(object);
            return object;
        }
    };
}

#define with(...)  WITH(__VA_ARGS__)
#define with_(...) WITH_(__VA_ARGS__) // Clang doesn't like lambdas having capture-defaults at namespace scope, so we have to have a separate macro for use at namespace scope.

#define WITH(...)  ->* ::Macro::WithExpr([&](auto &_object_) -> auto & { MA_VA_FOR_EACH(IMPL_WITH, MA_NULL, , __VA_ARGS__) return _object_; })
#define WITH_(...) ->* ::Macro::WithExpr([ ](auto &_object_) -> auto & { MA_VA_FOR_EACH(IMPL_WITH, MA_NULL, , __VA_ARGS__) return _object_; })

#define IMPL_WITH(i, data, expr) _object_.expr;
#define L__FUNCTION__
