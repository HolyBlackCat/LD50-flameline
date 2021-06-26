#pragma once

#include "program/platform.h"

// Portable wrappers for diagnostics pragmas.
// Should probably be used together with `platform.h`.
// Example:
//     DIAGNOSTICS_PUSH
//     DIAGNOSTICS_IGNORE("-Wextra-semi") // GCC or Clang.
//     DIAGNOSTICS_IGNORE(4001) // MSVC.
//     // Do things.
//     DIAGNOSTICS_POP

#if !PLATFORM_IS(msvc) // GCC or Clang.
#  define DIAGNOSTICS_PUSH _Pragma("GCC diagnostic push")
#  define DIAGNOSTICS_POP  _Pragma("GCC diagnostic pop")
#  define DIAGNOSTICS_IGNORE(name) DIAGNOSTICS_IGNORE_impl(GCC diagnostic ignored name)
#  define DIAGNOSTICS_IGNORE_impl(value) _Pragma(#value)
#else // MSVC.
#  define DIAGNOSTICS_PUSH _Pragma("warning(push)")
#  define DIAGNOSTICS_POP  _Pragma("warning(pop")
#  define DIAGNOSTICS_IGNORE(id) DIAGNOSTICS_IGNORE_impl(warning(disable: id))
#  define DIAGNOSTICS_IGNORE_impl(value) _Pragma(#value)
#endif
