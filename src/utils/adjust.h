#pragma once

#include <type_traits>
#include <utility>

#include "utils/macro.h"

namespace Macro
{
    template <typename F> class WithExpr
    {
        F lambda;

      public:
        constexpr WithExpr(F &&lambda) : lambda(std::move(lambda)) {}

        template <typename P> friend auto operator->*(P param, WithExpr &&expr)
        {
            expr.lambda(param);
            return param;
        }
    };

    template <typename T, typename F> class WithExprTyped
    {
        F lambda;

      public:
        constexpr WithExprTyped(F &&lambda) : lambda(std::move(lambda)) {}

        template <typename P> friend T operator->*(P &&param, WithExprTyped &&expr)
        {
            T object(std::forward<P>(param));
            expr.lambda(object);
            return static_cast<T>(object);
        }
    };

    template <typename T, typename F> auto MakeWithExpr(F &&lambda)
    {
        if constexpr (std::is_void_v<T>)
            return WithExpr<std::decay_t<F>>(std::forward<F>(lambda));
        else
            return WithExprTyped<T, std::decay_t<F>>(std::forward<F>(lambda));
    }
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

#define IMPL_ADJUST(capture, object, ...) (void(), [capture]{ auto _object_ = object; MA_VA_FOR_EACH(IMPL_ADJUST_STEP, MA_NULL, , __VA_ARGS__) return _object_; }())
#define IMPL_ADJUST_STEP(i, data, expr) _object_.expr;

#define IMPL_WITH(capture, ...) MA_CAT(IMPL_WITH_LOW_, MA_VA_SIZE(MA_CAT(IMPL_WITH_CHECK_, MA_VA_FIRST(__VA_ARGS__))))(capture, __VA_ARGS__)

#define IMPL_WITH_CHECK_struct ,
#define IMPL_WITH_CHECK_class ,
#define IMPL_WITH_CHECK_typename ,
#define IMPL_WITH_CHECK_NULL_struct
#define IMPL_WITH_CHECK_NULL_class
#define IMPL_WITH_CHECK_NULL_typename

#define IMPL_WITH_LOW_1(capture, ...) IMPL_WITH_LOW(capture, void, auto, __VA_ARGS__)
#define IMPL_WITH_LOW_2(capture, type, ...) IMPL_WITH_LOW(capture, MA_CAT(IMPL_WITH_CHECK_NULL_, type), MA_CAT(IMPL_WITH_CHECK_NULL_, type), __VA_ARGS__)

#define IMPL_WITH_LOW(capture, template_param, lambda_param, ...) ->* ::Macro::MakeWithExpr<template_param>([capture](lambda_param &_object_) { MA_VA_FOR_EACH(IMPL_ADJUST_STEP, MA_NULL, , __VA_ARGS__) })
