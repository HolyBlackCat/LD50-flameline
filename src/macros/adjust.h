#pragma once

#include <type_traits>
#include <utility>

#include "macros/generated.h"

namespace Macro
{
    template <typename F> class WithExpr
    {
        F &&lambda;

      public:
        constexpr WithExpr(F &&lambda) : lambda(std::move(lambda)) {}

        template <typename P> friend decltype(auto) operator->*(P &&param, WithExpr &&expr)
        {
            auto copy = std::forward<P>(param); // We make a copy here, rather than by passing `param` by value, to enable NRVO.
            expr.lambda(copy);
            return copy;
        }
    };
}

// Alternative macros with `_` suffix should always be used at namespace scope.
// Those macros use lambdas without default captures, since Clang refuses to handle those at namespace scope.

#define adjust(...) ADJUST(__VA_ARGS__)
#define adjust_(...) ADJUST_(__VA_ARGS__)
#define with(...) WITH(__VA_ARGS__)
#define with_(...) WITH_(__VA_ARGS__)

#define ADJUST(object, ...) IMPL_ADJUST(&, object, __VA_ARGS__)
#define ADJUST_(object, ...) IMPL_ADJUST(, object, __VA_ARGS__)
#define WITH(...) IMPL_WITH(&, __VA_ARGS__)
#define WITH_(...) IMPL_WITH(, __VA_ARGS__)

#define IMPL_ADJUST(capture, object, ...) (void(), [capture]{ auto _object_ = object; MA_VA_FOR_EACH(,IMPL_ADJUST_STEP,MA_TR_C(__VA_ARGS__)) return _object_; }())
#define IMPL_ADJUST_STEP(data, i, expr) _object_.expr;

#define IMPL_WITH(capture, ...) ->* ::Macro::WithExpr([capture](auto &_object_) { MA_VA_FOR_EACH(,IMPL_ADJUST_STEP,MA_TR_C(__VA_ARGS__)) })
