#pragma once

/* Example usage:
 *   MAYBE_CONST( CV int &GetX() CV {...} )
 * This expands to:
 *         int &GetX()       {...}
 *   const int &GetX() const {...}
 *
 * Note that if `CV` appears inside of parentheses, those parentheses must be preceeded by `CV_IN`.
 * Example:
 *   MAYBE_CONST( CV int &Foo CV_IN(CV int &x); )
 * This expands to:
 *         int &Foo(      int &x);
 *   const int &Foo(const int &x);
 */
#define MAYBE_CONST(...) IMPL_CV_maybe_const( (IMPL_CV_null,__VA_ARGS__)() )
#define CV )(IMPL_CV_identity,
#define CV_IN(...) )(IMPL_CV_p_open,)(IMPL_CV_null,__VA_ARGS__)(IMPL_CV_p_close,)(IMPL_CV_null,

#define IMPL_CV_null(...)
#define IMPL_CV_identity(...) __VA_ARGS__
#define IMPL_CV_p_open(...) (
#define IMPL_CV_p_close(...) )

#define IMPL_CV_maybe_const(seq) IMPL_CV_a seq IMPL_CV_const_a seq

#define IMPL_CV_body(cv, m, ...) m(cv) __VA_ARGS__

#define IMPL_CV_a(...) __VA_OPT__(IMPL_CV_body(,__VA_ARGS__) IMPL_CV_b)
#define IMPL_CV_b(...) __VA_OPT__(IMPL_CV_body(,__VA_ARGS__) IMPL_CV_a)

#define IMPL_CV_const_a(...) __VA_OPT__(IMPL_CV_body(const,__VA_ARGS__) IMPL_CV_const_b)
#define IMPL_CV_const_b(...) __VA_OPT__(IMPL_CV_body(const,__VA_ARGS__) IMPL_CV_const_a)
