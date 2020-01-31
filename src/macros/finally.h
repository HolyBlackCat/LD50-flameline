#pragma once

#include <exception>
#include <utility>

namespace Macro
{
    template <typename T> class FinallyObject
    {
        T func;

      public:
        FinallyObject(T &&func) : func(std::move(func)) {}
        FinallyObject(const FinallyObject &) = delete;
        FinallyObject &operator=(const FinallyObject &) = delete;
        ~FinallyObject()
        {
            func();
        }
    };
}

#define FINALLY_impl_cat(a, b) FINALLY_impl_cat_(a, b)
#define FINALLY_impl_cat_(a, b) a##b

#define FINALLY(...) ::Macro::FinallyObject FINALLY_impl_cat(_finally_object_,__LINE__) ([&]{ __VA_ARGS__ });
#define FINALLY_ON_THROW(...) FINALLY( if (::std::uncaught_exceptions()) { __VA_ARGS__ } )
