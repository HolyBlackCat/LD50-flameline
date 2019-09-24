#pragma once

// CGLFL (configurable OpenGL function loader)
//   by Egor Mikhailov <blckcat@inbox.ru>


// You can tweak this file according to your needs.

// Don't allocate the default context.
// If this is enabled, you need to manually set `cglfl::context_pointer`.
// #define CGLFL_NO_DEFAULT_CONTEXT

// Enable debugging features, such as logging and automatic error checking.
// #define CGLFL_DEBUG

// Use a custom 'to string' conversion function for logging.
// #define CGLFL_DEBUG_TO_STRING ::cglfl::debug::to_string

// When debugging is enabled, override additional code that is inserted into functions.
// #define CGLFL_DEBUG_PRE(i, func, ret, n, param_names, params) ...
// #define CGLFL_DEBUG_POST(i, func, ret, n, param_names, params) ...

// Override function call macro. Use with care.
// #define CGLFL_CALL ...
