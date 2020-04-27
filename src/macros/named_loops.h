#pragma once

/* Named loops.
 *
 * This file provides a few simple macros, adding support for so-called "named loops" (similar to what Java has).
 * Naming a loop allows to to `break` or `continue` it from a nested loop.
 *
 * Example usage:
 *
 *     for (int i = 0; i < 3; i++) LOOP_NAME(foo) // Loop name is specified here.
 *     {
 *         for (int j = 0; j < 3; j++)
 *         {
 *             std::cout << i << ' ' << j << '\n';
 *             if (i == 1 && j == 1)
 *                 break(foo); // Breaks from the outer loop.
 *             }
 *         }
 *     }
 *
 * `continue` works in a similar fashion.
 *
 * `LOOP_NAME` can be used with all types of loops (`for`, `while`, and `do-while`).
 * For `do-while`, `LOOP_NAME` must be placed immediately after `do`.
 * For all loop types, the body doesn't have to be enclosed in braces.
 *
 * Those macros are fairly robust: if the loop name specified in `break`/`continue` is invalid,
 * or doesn't enclose the current statement, you get an error.
 */

#define LOOP_NAME(name) \
    /* The variable in the conditions prevents `BREAK/CONTINUE` from */\
    /* being used outside of the loop with the matching name. */\
    if ([[maybe_unused]] constexpr bool _namedloop_InvalidBreakOrContinue = false) \
    { \
        [[maybe_unused]] MA_CAT(_namedloop_break_,name): break; \
        [[maybe_unused]] MA_CAT(_namedloop_continue_,name): continue; \
    } \
    else

#define BREAK(name) goto MA_CAT(_namedloop_break_,name)
#define CONTINUE(name) goto MA_CAT(_namedloop_continue_,name)

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
#define break(x) BREAK(x)
#define continue(x) CONTINUE(x)
#ifdef __clang__
#pragma clang diagnostic pop
#endif
