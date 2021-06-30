#pragma once

#include "program/platform.h"

#if !IMP_PLATFORM_IS(msvc) // GCC or Clang.
// Portable wrappers for diagnostics pragmas.
// Should probably be used together with `platform.h`.
// Example:
//     IMP_DIAGNOSTICS_PUSH
//     IMP_DIAGNOSTICS_IGNORE("-Wextra-semi") // GCC or Clang.
//     IMP_DIAGNOSTICS_IGNORE(4001) // MSVC.
//     // Do things.
//     IMP_DIAGNOSTICS_POP
#  define IMP_DIAGNOSTICS_PUSH _Pragma("GCC diagnostic push")
#  define IMP_DIAGNOSTICS_POP  _Pragma("GCC diagnostic pop")
#  define IMP_DIAGNOSTICS_IGNORE(name) IMP_DIAGNOSTICS_IGNORE_impl(GCC diagnostic ignored name)
#  define IMP_DIAGNOSTICS_IGNORE_impl(value) _Pragma(#value)
// Causes a function to always be inlined.
#  define IMP_ALWAYS_INLINE __attribute__((__always_inline__))
#else // MSVC.
#  define IMP_DIAGNOSTICS_PUSH _Pragma("warning(push)")
#  define IMP_DIAGNOSTICS_POP  _Pragma("warning(pop")
#  define IMP_DIAGNOSTICS_IGNORE(id) IMP_DIAGNOSTICS_IGNORE_impl(warning(disable: id))
#  define IMP_DIAGNOSTICS_IGNORE_impl(value) _Pragma(#value)
#  define IMP_ALWAYS_INLINE __forceinline
#endif
