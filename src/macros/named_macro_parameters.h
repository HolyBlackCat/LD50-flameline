#pragma once

#include "macros/generated.h"

/* Support macros for mamed macro parameters.
 *
 * Usage:
 *
 * First, you have to declare parameter names. E.g:
 *
 *   #define ALPHA MA_PARAM(Alpha)
 *   #define MA_PARAMS_category_MyParams_X_Alpha
 *   #define MA_PARAMS_equal_Alpha_X_Alpha
 *
 *   #define BETA MA_PARAM(Beta)
 *   #define MA_PARAMS_category_MyParams_X_Beta
 *   #define MA_PARAMS_equal_Beta_X_Beta
 *
 * Here, parameters are named `Alpha` and `Beta`. To simplify their use, `ALPHA` and `BETA` macros are also created.
 * Those parameters belong to a category named `MyParams`.
 * Parameter names and category names must not start or end with an underscore (`_`), and must not contain `_X_` sequence.
 *
 * Now, you can use these parameters in the following way:
 *   FOO(zero ALPHA a BETA b ALPHA c BETA d)
 * Here `FOO` should be defined as `FOO(...)` (or `FOO(x)` if the arguments can't have commas in them).
 *
 * You can extract the first (unnamed) parameter from a list using
 *   MA_PARAMS_FIRST( ((__VA_ARGS__)) )
 * Note that parameter lists always have to be wrapped in `((...))`. We can't wrap them inside of the macros, since
 * they are highly unstable otherwise, and can't even be passed to a nested macro without the `((...))`.
 *
 * You can extract all parameters with a specific name from a list using
 *   MA_PARAMS_GET(data, category, param_name, ((__VA_ARGS__)), macro)`
 * Where `data` is a custom user data, `category` is a parameter name category (`MyParams` in this example),
 * `param_name` is a parameter name (e.g. `Alpha` in this example), `macro` is a macro that will be called for each
 * parameter with the matching name.
 *
 * If at least one provided parameter has a category different from the one specified, an error occurs.
 * `macro` is called as `macro(data, ...)`, where `data` is the custom user data, and `...` are the parameter contents.
 *
 * If you want to disallow duplicate parameters, use
 *   MA_PARAMS_GET_ONE(category, param_name, ((__VA_ARGS__)) )
 * instead. It expands to the matching parameter, or to nothing if there are none.
 * If there are several matching parameters, an error is emitted.
 */

#define MA_PARAM(name) ))(name,(

// Obtains the first parameter (the unnamed one) from the `list`.
// The list has to be wrapped in `((...))`.
#define MA_PARAMS_FIRST(list) MA_SEQ_FIRST( MA_IDENTITY list )

// Calls the `macro` for each parameter with the matching name in the `list`.
// The list has to be wrapped in `((...))`.
// `data` is forwarded to the provided macro. `param` is the parameter name.
// `category` is the parameter category name. If at least one provided parameter doesn't belong to this category, an error is emitted.
// `macro` is called as `macro(data, ...)`, where `...` is the matching parameter.
#define MA_PARAMS_GET(data, category, param, list, macro) \
    MA_SEQ_FOR_EACH((data, category, param, macro), MA_PARAMS_GET_impl, MA_SEQ_NO_FIRST(list))

#define MA_PARAMS_GET_impl(elemdata, index, curparam, curvalue) \
    MA_PARAMS_GET_impl_call(MA_PARAMS_GET_impl0, MA_IDENTITY elemdata, curparam, curvalue)


#define MA_PARAMS_GET_impl0(data, category, param, macro, curparam, curvalue) \
    MA_PARAMS_GET_impl_verify(category, curparam) \
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, macro, MA_CAT4(MA_PARAMS_equal_,param,_X_,curparam))(data, MA_IDENTITY curvalue)

#define MA_PARAMS_GET_impl_call(m, ...) m(__VA_ARGS__) // Used instead of `MA_CALL` since it's not reentrant.

#define MA_PARAMS_GET_impl_verify(category, param) \
    MA_IF_NOT_EMPTY(MA_ABORT("Invalid argument for a named macro parameter."), MA_CAT4(MA_PARAMS_category_,category,_X_,param))

// Similar to `MA_PARAMS_GET`, but expects at most one parameter with a matching name.
// Expands to the matching parameter (or nothing, if there is none).
// Emits an error if there is more than one matching parameter.
#define MA_PARAMS_GET_ONE(data, category, param, list, macro) \
    MA_IDENTITY(MA_NULL MA_SEQ_FOR_EACH((data, category, param, macro), MA_PARAMS_GET_ONE_impl, MA_SEQ_NO_FIRST(list)) ())

#define MA_PARAMS_GET_ONE_impl(elemdata, index, curparam, curvalue) \
    MA_PARAMS_GET_impl_call(MA_PARAMS_GET_ONE_impl0, MA_IDENTITY elemdata, curparam, curvalue)

#define MA_PARAMS_GET_ONE_impl0(data, category, param, macro, curparam, curvalue) \
    MA_PARAMS_GET_impl_verify(category, curparam) \
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, MA_PARAMS_GET_ONE_impl1, MA_CAT4(MA_PARAMS_equal_,param,_X_,curparam))(data, macro, MA_IDENTITY curvalue)

#define MA_PARAMS_GET_ONE_impl1(data, macro, ...) (x) macro(data, __VA_ARGS__) MA_PARAMS_GET_ONE_impl_dupe

#define MA_PARAMS_GET_ONE_impl_dupe(...) __VA_OPT__(MA_ABORT("Duplicate named macro parameter."))

// This can be passed as `macro` to `MA_PARAMS_GET[_ONE]`.
// Expands to the parameter.
#define MA_PARAMS_IDENTITY(data, ...) __VA_ARGS__

// This can be passed as `macro` to `MA_PARAMS_GET[_ONE]`.
// Expands to the parameter enclosed in `(...)`.
#define MA_PARAMS_PARENS(data, ...) (__VA_ARGS__)

// This can be passed as `macro` to `MA_PARAMS_GET[_ONE]`.
// Expands to a single `x`.
#define MA_PARAMS_DUMMY(data, ...) x

// This can be passed as `macro` to `MA_PARAMS_GET[_ONE]`.
// Expands to a single comma.
// Emits an error if the parameter is not empty.
#define MA_PARAMS_DUMMY_EMPTY(data, ...) x __VA_OPT__(MA_ABORT("This named parameter has to be empty"))
