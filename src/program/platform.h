#pragma once

// This file contains macros describing platform properties.

// Platform properties are described by macro flags named `IMP_PLATFORM_FLAG_<flag>`, defined to `0` or `1`.
// `IMP_PLATFORM_IF(flag)(text)` expands to `text` if the flag `IMP_PLATFORM_FLAG_<flag>` is `1`, and to
// nothing if the flag is `0`. If the flag has some other value or is not defined, you get a compile-time error
// (along the lines of "Too few arguments provided to function-like macro invocation".)
// `IMP_PLATFORM_IF_NOT(flag)(text)` works in a similar way, but inverts the flag.
// `IMP_PLATFORM_IS(flag)` reads the value of the flag (`0` or `1`), or generates a error if the flag has some other value or is not defined.
#define IMP_PLATFORM_IF(flag) IMP_PLATFORM_IF_impl_eval(IMP_PLATFORM_IF_impl_cat(IMP_PLATFORM_IF_impl_if_, IMP_PLATFORM_IF_impl_cat(IMP_PLATFORM_FLAG_, flag)))
#define IMP_PLATFORM_IF_NOT(flag) IMP_PLATFORM_IF_impl_eval(IMP_PLATFORM_IF_impl_cat(IMP_PLATFORM_IF_impl_ifnot_, IMP_PLATFORM_IF_impl_cat(IMP_PLATFORM_FLAG_, flag)))
#define IMP_PLATFORM_IS(flag) IMP_PLATFORM_IF(flag)(1) IMP_PLATFORM_IF_NOT(flag)(0)

#define IMP_PLATFORM_IF_impl_eval(...) IMP_PLATFORM_IF_impl_eval_(__VA_ARGS__)
#define IMP_PLATFORM_IF_impl_eval_(macro, dummy) macro

#define IMP_PLATFORM_IF_impl_cat(x,y) IMP_PLATFORM_IF_impl_cat_(x,y)
#define IMP_PLATFORM_IF_impl_cat_(x,y) x##y

#define IMP_PLATFORM_IF_impl_false(...)
#define IMP_PLATFORM_IF_impl_true(...) __VA_ARGS__
#define IMP_PLATFORM_IF_impl_if_0 IMP_PLATFORM_IF_impl_false,
#define IMP_PLATFORM_IF_impl_if_1 IMP_PLATFORM_IF_impl_true,
#define IMP_PLATFORM_IF_impl_ifnot_0 IMP_PLATFORM_IF_impl_true,
#define IMP_PLATFORM_IF_impl_ifnot_1 IMP_PLATFORM_IF_impl_false,

// Built-in platform flags:

// - Endianness

#ifndef IMP_PLATFORM_FLAG_big_endian
#  if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#    define IMP_PLATFORM_FLAG_big_endian 1
#  else
#    define IMP_PLATFORM_FLAG_big_endian 0
#  endif
#endif

#ifndef IMP_PLATFORM_FLAG_little_endian
#  if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#    define IMP_PLATFORM_FLAG_little_endian 1
#  else
#    define IMP_PLATFORM_FLAG_little_endian 0
#  endif
#endif

#if IMP_PLATFORM_IS(big_endian) + IMP_PLATFORM_IS(little_endian) != 1
#  error Invalid platform flags: Endianness.
#endif

// - Compilers

#ifndef IMP_PLATFORM_FLAG_gcc
#  if defined __GNUC__ && !defined __clang__
#    define IMP_PLATFORM_FLAG_gcc 1
#  else
#    define IMP_PLATFORM_FLAG_gcc 0
#  endif
#endif

#ifndef IMP_PLATFORM_FLAG_clang
#  if defined __clang__
#    define IMP_PLATFORM_FLAG_clang 1
#  else
#    define IMP_PLATFORM_FLAG_clang 0
#  endif
#endif

#ifndef IMP_PLATFORM_FLAG_msvc
#  if defined _MSC_VER
#    define IMP_PLATFORM_FLAG_msvc 1
#  else
#    define IMP_PLATFORM_FLAG_msvc 0
#  endif
#endif

// Set this manually if you compile using an unknown compiler.
#ifndef IMP_PLATFORM_FLAG_unknown_compiler
#  define IMP_PLATFORM_FLAG_unknown_compiler 0
#endif

#if IMP_PLATFORM_IS(gcc) + IMP_PLATFORM_IS(clang) + IMP_PLATFORM_IS(msvc) + IMP_PLATFORM_IS(unknown_compiler) != 1
#  error Invalid platform flags: Compiler.
#endif

// - Compiler categories

#if IMP_PLATFORM_IS(gcc) || IMP_PLATFORM_IS(clang)
#  define IMP_PLATFORM_FLAG_gcc_clang 1
#else
#  define IMP_PLATFORM_FLAG_gcc_clang 0
#endif

// - Operating systems

#ifndef IMP_PLATFORM_FLAG_windows
#  if defined _WIN32 // It should be defined for x64 Windows as well.
#    define IMP_PLATFORM_FLAG_windows 1
#  else
#    define IMP_PLATFORM_FLAG_windows 0
#  endif
#endif

#ifndef IMP_PLATFORM_FLAG_macos
#  if defined __APPLE__
#    define IMP_PLATFORM_FLAG_macos 1
#  else
#    define IMP_PLATFORM_FLAG_macos 0
#  endif
#endif

#ifndef IMP_PLATFORM_FLAG_linux
#  if defined __linux__ && !defined __ANDROID__
#    define IMP_PLATFORM_FLAG_linux 1
#  else
#    define IMP_PLATFORM_FLAG_linux 0
#  endif
#endif

#ifndef IMP_PLATFORM_FLAG_android
#  if defined __ANDROID__
#    define IMP_PLATFORM_FLAG_android 1
#  else
#    define IMP_PLATFORM_FLAG_android 0
#  endif
#endif

// Set this manually if you compile for an unknown OS.
#ifndef IMP_PLATFORM_FLAG_unknown_os
#  define IMP_PLATFORM_FLAG_unknown_os 0
#endif

#if IMP_PLATFORM_IS(windows) + IMP_PLATFORM_IS(macos) + IMP_PLATFORM_IS(linux) + IMP_PLATFORM_IS(android) + IMP_PLATFORM_IS(unknown_os) != 1
#  error Invalid platform flags: OS.
#endif

// - Operating system categories

#ifndef IMP_PLATFORM_FLAG_pc
#  if IMP_PLATFORM_IS(windows) || IMP_PLATFORM_IS(macos) || IMP_PLATFORM_IS(linux)
#    define IMP_PLATFORM_FLAG_pc 1
#  else
#    define IMP_PLATFORM_FLAG_pc 0
#  endif
#endif

#ifndef IMP_PLATFORM_FLAG_mobile
#  if IMP_PLATFORM_IS(android)
#    define IMP_PLATFORM_FLAG_mobile 1
#  else
#    define IMP_PLATFORM_FLAG_mobile 0
#  endif
#endif

#if IMP_PLATFORM_IS(pc) + IMP_PLATFORM_IS(mobile) > 1
#  error Invalid platform flags: More than one OS category is specified.
#endif
