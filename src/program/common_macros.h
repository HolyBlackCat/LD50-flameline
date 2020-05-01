#pragma once

// This file is automatically included in every source file.
// You shouldn't need to include it manually.

// Even though we define this macro, we still use `SDL_main` on platforms that need it.
// We determine if it's needed or not using the flags obtained from `pkg-config`, so
// `#define main` (that we'd get if we didn't define `SDL_MAIN_HANDLED`) would
// just get in the way, without giving us any new information.
#define SDL_MAIN_HANDLED
