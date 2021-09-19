#pragma once

// Defines the `IMP_MAIN(argc, argv)` macro, which should be used as the entry point, instead of `int main(int argc, char **argv)`.
// Parameter names can be empty if unused.
// The macro includes the return type and the parameter types to enforce the correct types, which are important at least on Windows.

// The function name is `main` by default, but can be changed with `IMP_ENTRY_POINT_OVERRIDE`.

#ifdef IMP_ENTRY_POINT_OVERRIDE
#  define IMP_MAIN(argc, argv) int IMP_ENTRY_POINT_OVERRIDE(int argc, char **argv)
#else
#  define IMP_MAIN(argc, argv) int main(int argc, char **argv)
#endif
