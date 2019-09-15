#pragma once

#include <type_traits>
#include <utility>

#define CHECK(...) ::std::enable_if_t<__VA_ARGS__, decltype(nullptr)> = nullptr
#define CHECK_EXPR(...) decltype(void(__VA_ARGS__), nullptr) = nullptr
#define CHECK_TYPE(...) decltype(void(::std::declval<__VA_ARGS__>()), nullptr) = nullptr
