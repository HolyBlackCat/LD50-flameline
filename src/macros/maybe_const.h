#pragma once

/* Example usage:
 *   MAYBE_CONST( CV int &GetX() CV {...} )
 * This macro has a major limitation: CV can't appear inside of ().
 * I doubt a fix is possible.
 */

#define MAYBE_CONST(...) IMPL_MAYBE_CONST( ((__VA_ARGS__)) )
#define CV ))((
// Double parens are probably an overkill, but I want this to be extra robust.

#define IMPL_MAYBE_CONST(seq) \
    IMPL_MAYBE_CONST_end(IMPL_MAYBE_CONST_a seq) \
    IMPL_MAYBE_CONST_end(IMPL_MAYBE_CONST_const_0 seq)

#define IMPL_MAYBE_CONST_identity(...) __VA_ARGS__

#define IMPL_MAYBE_CONST_end(...) IMPL_MAYBE_CONST_end_(__VA_ARGS__)
#define IMPL_MAYBE_CONST_end_(...) __VA_ARGS__##_end

#define IMPL_MAYBE_CONST_a(elem) IMPL_MAYBE_CONST_identity elem IMPL_MAYBE_CONST_b
#define IMPL_MAYBE_CONST_b(elem) IMPL_MAYBE_CONST_identity elem IMPL_MAYBE_CONST_a
#define IMPL_MAYBE_CONST_a_end
#define IMPL_MAYBE_CONST_b_end

#define IMPL_MAYBE_CONST_const_0(elem)       IMPL_MAYBE_CONST_identity elem IMPL_MAYBE_CONST_const_a
#define IMPL_MAYBE_CONST_const_a(elem) const IMPL_MAYBE_CONST_identity elem IMPL_MAYBE_CONST_const_b
#define IMPL_MAYBE_CONST_const_b(elem) const IMPL_MAYBE_CONST_identity elem IMPL_MAYBE_CONST_const_a
// Note that we don't need `IMPL_MAYBE_CONST_const_0_end`, since the sequence is never empty.
#define IMPL_MAYBE_CONST_const_a_end
#define IMPL_MAYBE_CONST_const_b_end
