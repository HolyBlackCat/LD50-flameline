#pragma once

#define SDL_MAIN_HANDLED


/* How to override a platform flag?
 *
 * To set a custom flag (overriding builtin detection), do following
 *
 *     #define OVERRIDE_<flag_cagetory>
 *     #deiine PLATFORM_<flag>
 *
 * Example:
 *
 *     #define OVERRIDE_COMPILER
 *     #define PLATFORM_CLANG
 *
 * You can define `PLATFORM_*` to `1` rather than `` (empty). In fact, that's what `-D` flag does.
 */


// Platform type detection

#ifndef OVERRIDE_PLATFORM
#  if defined ANDROID || defined __ANDROID__
#    define PLATFORM_MOBILE 1
#  else
#    define PLATFORM_PC 1
#  endif
#endif


// Platform type detection

#ifndef OVERRIDE_PLATFORM
#  if defined _WIN32 // It should be defined for x64 Windows as well.
#    define PLATFORM_WINDOWS 1
#  elif defined __APPLE__
#    define PLATFORM_MACOS 1
#  else
#    define PLATFORM_LINUX 1
#  endif
#endif


// Compiler detection

#ifndef OVERRIDE_COMPILER
#  if defined __clang__
#    define PLATFORM_CLANG 1
#  elif defined __GNUC__ // Clang also defines this, so we check for Clang first.
#    define PLATFORM_GCC 1
#  elif defined _MSC_VER
#    define PLATFORM_MSVC 1
#  else
#    error Unable to detect compiler.
#  endif
#endif


// Byte order detection

#include <SDL2/SDL_endian.h>

#ifndef OVERRIDE_ENDIANNESS
#  if SDL_BYTEORDER == SDL_BIG_ENDIAN
#    define PLATFORM_BIG_ENDIAN 1
#  elif SDL_BYTEORDER == SDL_LIL_ENDIAN
#    define PLATFORM_LITTLE_ENDIAN 1
#  else
#    error Invalid endianness.
#  endif
#endif


// Conditional macros

/* Usage:
 *
 *     [Not]OnPlatform(<flag>)(<code>)
 *
 * Example:
 *
 *     OnPlatform(GCC)( std::cout << "I'm on GCC!\n"; )
 *     NotOnPlatform(GCC)( std::cout << "I'm not on GCC!\n"; )
 *     bool is_on_gcc = IsOnPlatform(GCC);
 *     bool is_on_gcc = IsOnPlatform(GCC);
 *
 */
#define OnPlatform(flag)      PLATFORM_IMPL_CONDITION(flag, PLATFORM_IMPL_IDENTITY, PLATFORM_IMPL_NULL)
#define NotOnPlatform(flag)   PLATFORM_IMPL_CONDITION(flag, PLATFORM_IMPL_NULL, PLATFORM_IMPL_IDENTITY)
#define IsOnPlatform(flag)    PLATFORM_IMPL_CONDITION(flag, true, false)
#define IsNotOnPlatform(flag) PLATFORM_IMPL_CONDITION(flag, false, true)

#define PLATFORM_IMPL_CONDITION(flag, equal, not_equal) PLATFORM_IMPL_CONDITION_A(PLATFORM_IMPL_CAT(PLATFORM_IMPL_CAT(PLATFORM_IMPL_COMMA_, PLATFORM_IMPL_CAT(PLATFORM_, flag)),END), equal, not_equal)
#define PLATFORM_IMPL_CONDITION_A(cond, equal, not_equal) PLATFORM_IMPL_CONDITION_B(cond, equal, not_equal,)
#define PLATFORM_IMPL_CONDITION_B(a, b, c, ...) c
#define PLATFORM_IMPL_COMMA_END ,
#define PLATFORM_IMPL_COMMA_1END , // We need this for convenience, since `-D` flag defines macros as `1`.
#define PLATFORM_IMPL_CAT(a, b) PLATFORM_IMPL_CAT_(a, b)
#define PLATFORM_IMPL_CAT_(a, b) a##b
#define PLATFORM_IMPL_IDENTITY(...) __VA_ARGS__
#define PLATFORM_IMPL_NULL(...)
