#pragma once

// This file contains macros describing platform properties.

// Platform properties are described by macro flags named `PLATFORM_FLAG_<flag>`, defined to `0` or `1`.
// `PLATFORM_IF(flag)(text)` expands to `text` if the flag `PLATFORM_FLAG_<flag>` is `1`, and to
// nothing if the flag is `0`. If the flag has some other value or is not defined, you get a compile-time error
// (along the lines of "Too few arguments provided to function-like macro invocation".)
// `PLATFORM_IF_NOT(flag)(text)` works in a similar way, but inverts the flag.
// `PLATFORM_IS(flag)` reads the value of the flag (`0` or `1`), or generates a error if the flag has some other value or is not defined.
#define PLATFORM_IF(flag) PLATFORM_IF_impl_eval(PLATFORM_IF_impl_cat(PLATFORM_IF_impl_if_, PLATFORM_IF_impl_cat(PLATFORM_FLAG_, flag)))
#define PLATFORM_IF_NOT(flag) PLATFORM_IF_impl_eval(PLATFORM_IF_impl_cat(PLATFORM_IF_impl_ifnot_, PLATFORM_IF_impl_cat(PLATFORM_FLAG_, flag)))
#define PLATFORM_IS(flag) PLATFORM_IF(flag)(1) PLATFORM_IF_NOT(flag)(0)

#define PLATFORM_IF_impl_eval(...) PLATFORM_IF_impl_eval_(__VA_ARGS__)
#define PLATFORM_IF_impl_eval_(macro, dummy) macro

#define PLATFORM_IF_impl_cat(x,y) PLATFORM_IF_impl_cat_(x,y)
#define PLATFORM_IF_impl_cat_(x,y) x##y

#define PLATFORM_IF_impl_false(...)
#define PLATFORM_IF_impl_true(...) __VA_ARGS__
#define PLATFORM_IF_impl_if_0 PLATFORM_IF_impl_false,
#define PLATFORM_IF_impl_if_1 PLATFORM_IF_impl_true,
#define PLATFORM_IF_impl_ifnot_0 PLATFORM_IF_impl_true,
#define PLATFORM_IF_impl_ifnot_1 PLATFORM_IF_impl_false,

// Built-in platform flags:

// - Endianness

#ifndef PLATFORM_FLAG_big_endian
#  if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#    define PLATFORM_FLAG_big_endian 1
#  else
#    define PLATFORM_FLAG_big_endian 0
#  endif
#endif

#ifndef PLATFORM_FLAG_little_endian
#  if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#    define PLATFORM_FLAG_little_endian 1
#  else
#    define PLATFORM_FLAG_little_endian 0
#  endif
#endif

#if PLATFORM_IS(big_endian) + PLATFORM_IS(little_endian) != 1
#  error Invalid platform flags: Endianness.
#endif

// - Compilers

#ifndef PLATFORM_FLAG_gcc
#  if defined __GNUC__ && !defined __clang__
#    define PLATFORM_FLAG_gcc 1
#  else
#    define PLATFORM_FLAG_gcc 0
#  endif
#endif

#ifndef PLATFORM_FLAG_clang
#  if defined __clang__
#    define PLATFORM_FLAG_clang 1
#  else
#    define PLATFORM_FLAG_clang 0
#  endif
#endif

#ifndef PLATFORM_FLAG_msvc
#  if defined _MSC_VER
#    define PLATFORM_FLAG_msvc 1
#  else
#    define PLATFORM_FLAG_msvc 0
#  endif
#endif

// Set this manually if you compile using an unknown compiler.
#ifndef PLATFORM_FLAG_unknown_compiler
#  define PLATFORM_FLAG_unknown_compiler 0
#endif

#if PLATFORM_IS(gcc) + PLATFORM_IS(clang) + PLATFORM_IS(msvc) + PLATFORM_IS(unknown_compiler) != 1
#  error Invalid platform flags: Compiler.
#endif

// - Operating systems

#ifndef PLATFORM_FLAG_windows
#  if defined _WIN32 // It should be defined for x64 Windows as well.
#    define PLATFORM_FLAG_windows 1
#  else
#    define PLATFORM_FLAG_windows 0
#  endif
#endif

#ifndef PLATFORM_FLAG_macos
#  if defined __APPLE__
#    define PLATFORM_FLAG_macos 1
#  else
#    define PLATFORM_FLAG_macos 0
#  endif
#endif

#ifndef PLATFORM_FLAG_linux
#  if defined __linux__ && !defined __ANDROID__
#    define PLATFORM_FLAG_linux 1
#  else
#    define PLATFORM_FLAG_linux 0
#  endif
#endif

#ifndef PLATFORM_FLAG_android
#  if defined __ANDROID__
#    define PLATFORM_FLAG_android 1
#  else
#    define PLATFORM_FLAG_android 0
#  endif
#endif

// Set this manually if you compile for an unknown OS.
#ifndef PLATFORM_FLAG_unknown_os
#  define PLATFORM_FLAG_unknown_os 0
#endif

#if PLATFORM_IS(windows) + PLATFORM_IS(macos) + PLATFORM_IS(linux) + PLATFORM_IS(android) + PLATFORM_IS(unknown_os) != 1
#  error Invalid platform flags: OS.
#endif

// - Operating system categories

#ifndef PLATFORM_FLAG_pc
#  if PLATFORM_IS(windows) || PLATFORM_IS(macos) || PLATFORM_IS(linux)
#    define PLATFORM_FLAG_pc 1
#  else
#    define PLATFORM_FLAG_pc 0
#  endif
#endif

#ifndef PLATFORM_FLAG_mobile
#  if PLATFORM_IS(android)
#    define PLATFORM_FLAG_mobile 1
#  else
#    define PLATFORM_FLAG_mobile 0
#  endif
#endif

#if PLATFORM_IS(pc) + PLATFORM_IS(mobile) > 1
#  error Invalid platform flags: More than one OS category is specified.
#endif
