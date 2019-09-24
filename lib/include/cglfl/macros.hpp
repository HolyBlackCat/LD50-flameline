#pragma once

// CGLFL (configurable OpenGL function loader)
//   by Egor Mikhailov <blckcat@inbox.ru>
// License: zlib

#include "config.hpp"

#ifndef CGLFL_CALL
#  ifndef CGLFL_DEBUG
#    define CGLFL_CALL CGLFL_CALL_DEFAULT
#  else
#    define CGLFL_CALL CGLFL_CALL_DEBUG
#  endif
#endif

#define CGLFL_CALL_DEFAULT(i, func, ret, n, param_names, params) \
    (void(), reinterpret_cast<ret (CGLFL_API *) params>(::cglfl::context_pointer->functions[i]))

#define CGLFL_CALL_DEBUG(i, func, ret, n, param_names, params) \
    (void(), +[] params -> ret { \
        CGLFL_DEBUG_PRE(i,func,ret,n,param_names,params) \
        CGLFL_IMPL_NOT_VOID(ret, auto _cglfl_result = ) CGLFL_CALL_DEFAULT(i,func,ret,n,param_names,params) param_names; \
        CGLFL_DEBUG_POST(i,func,ret,n,param_names,params) \
        CGLFL_IMPL_NOT_VOID(ret, return _cglfl_result;) \
    })

#ifndef CGLFL_DEBUG_PRE
#  define CGLFL_DEBUG_PRE(i, func, ret, n, param_names, params) \
    CGLFL_DEBUG_PRINT_PARAMS(i, func, ret, n, param_names, params)
#endif
#ifndef CGLFL_DEBUG_POST
#  define CGLFL_DEBUG_POST(i, func, ret, n, param_names, params) \
    CGLFL_DEBUG_PRINT_RET(i, func, ret, n, param_names, params) \
    CGLFL_DEBUG_CHECK_ERRORS(i, func, ret, n, param_names, params)
#endif

#define CGLFL_DEBUG_PRINT_PARAMS(i, func, ret, n, param_names, params) \
    if (::cglfl::debug::log_calls) \
    { \
        ::cglfl::debug::log(#func); \
        ::cglfl::debug::log("    :: at " __FILE__ " : " CGLFL_IMPL_STR(__LINE__)); \
        CGLFL_IMPL_FOR_EACH(CGLFL_DEBUG_PRINT_PARAM, n, CGLFL_IMPL_IDENTITY param_names) \
    }
#define CGLFL_DEBUG_PRINT_RET(i, func, ret, n, param_names, params) \
    CGLFL_IMPL_NOT_VOID(ret, \
    if (::cglfl::debug::log_calls) \
        ::cglfl::debug::log("    -> " + CGLFL_DEBUG_TO_STRING(_cglfl_result)); \
    )

#define CGLFL_DEBUG_CHECK_ERRORS(i, func, ret, n, param_names, params) \
    if (::cglfl::debug::check_errors) \
        ::cglfl::debug::check_errors_now();

#define CGLFL_DEBUG_PRINT_PARAM(param) \
    ::cglfl::debug::log("    " #param " = " + CGLFL_DEBUG_TO_STRING(param));

#ifndef CGLFL_DEBUG_TO_STRING
#  define CGLFL_DEBUG_TO_STRING ::cglfl::debug::to_string
#endif

#define CGLFL_IMPL_NOT_VOID(x, ...) CGLFL_IMPL_CALL(CGLFL_IMPL_NOT_VOID_, CGLFL_IMPL_CAT(CGLFL_IMPL_CHECK_, x), (__VA_ARGS__),)
#define CGLFL_IMPL_NOT_VOID_(a, b, ...) CGLFL_IMPL_IDENTITY b
#define CGLFL_IMPL_IDENTITY(...) __VA_ARGS__
#define CGLFL_IMPL_STR(...) CGLFL_IMPL_STR_(__VA_ARGS__)
#define CGLFL_IMPL_STR_(...) #__VA_ARGS__
#define CGLFL_IMPL_CALL(m, ...) m(__VA_ARGS__)
#define CGLFL_IMPL_CAT(x, y) CGLFL_IMPL_CAT_(x, y)
#define CGLFL_IMPL_CAT_(x, y) x##y
#define CGLFL_IMPL_CHECK_void ,()
