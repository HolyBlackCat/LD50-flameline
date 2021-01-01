#pragma once

#include <exception>
#include <utility>

namespace Macro
{
    template <typename T> class ScopeGuard
    {
        T func;
      public:
        ScopeGuard(T &&func) : func(std::move(func)) {}
        ScopeGuard(const ScopeGuard &) = delete;
        ScopeGuard &operator=(const ScopeGuard &) = delete;
        ~ScopeGuard() {func();}
    };

    // Same as `ScopeGuard`, but can throw.
    template <typename T> class ScopeGuardExc
    {
        T func;
      public:
        ScopeGuardExc(T &&func) : func(std::move(func)) {}
        ScopeGuardExc(const ScopeGuardExc &) = delete;
        ScopeGuardExc &operator=(const ScopeGuardExc &) = delete;
        ~ScopeGuardExc() noexcept(false) {func();}
    };
}

#define FINALLY_impl_cat(a, b) FINALLY_impl_cat_(a, b)
#define FINALLY_impl_cat_(a, b) a##b

#define FINALLY(...) \
    ::Macro::ScopeGuard FINALLY_impl_cat(_finally_object_,__LINE__) \
    ([&]() -> void { __VA_ARGS__ });

#define FINALLY_ON_THROW(...) \
    ::Macro::ScopeGuard FINALLY_impl_cat(_finally_object_,__LINE__) \
    ([&, _finally_exc_depth_ = ::std::uncaught_exceptions()]() -> void { if (::std::uncaught_exceptions() > _finally_exc_depth_) {__VA_ARGS__} });

#define FINALLY_ON_SUCCESS(...) \
    ::Macro::ScopeGuardExc FINALLY_impl_cat(_finally_object_,__LINE__) \
    ([&, _finally_exc_depth_ = ::std::uncaught_exceptions()]() -> void { if (::std::uncaught_exceptions() <= _finally_exc_depth_) {__VA_ARGS__} });
