#pragma once

#include <type_traits>
#include <utility>

// Put one (or several) of those at the end of a template parameter list.
// Note the extra `()`s around `__VA_ARGS__` (except when it's a type).

// Check if an expression is valid and `true`.
#define CHECK(...) ::std::enable_if_t<(__VA_ARGS__), decltype(nullptr)> = nullptr
// Check if an expression is valid.
#define CHECK_EXPR(...) decltype((void)(__VA_ARGS__), nullptr) = nullptr
// Check if a type is valid.
#define CHECK_TYPE(...) decltype(void(::std::declval<__VA_ARGS__>()), nullptr) = nullptr
