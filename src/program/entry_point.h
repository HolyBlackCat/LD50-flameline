#pragma once

// Defines the `_main_` macro, which should be used as the entry point function name, instead of main.
// The signature should be: `int _main_(int, char **)`.

// It expands to `ENTRY_POINT_OVERRIDE` if it's defined, otherwise to `main`.
// `ENTRY_POINT_OVERRIDE` is supposed to be defined by the build system.
// We don't make the build system define `_main_` directly, because we want ensure that
// `_main_` can only used after including this header.

#ifdef ENTRY_POINT_OVERRIDE
#  define _main_ ENTRY_POINT_OVERRIDE
#else
#  define _main_ main
#endif
