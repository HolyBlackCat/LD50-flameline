#ifndef UTILS_FINALLY_H_INCLUDED
#define UTILS_FINALLY_H_INCLUDED

#include <exception>
#include <utility>

#include "macro.h"

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

#define FINALLY(...) ::FinallyObject MA_CAT(_finally_object_,__LINE__) ([&]{ __VA_ARGS__ });
#define FINALLY_ON_THROW(...) FINALLY( if (::std::uncaught_exceptions()) { __VA_ARGS__ } )

#endif
