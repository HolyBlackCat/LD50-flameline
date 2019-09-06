#pragma once

#include <type_traits>

#define CHECK(...) ::std::enable_if_t<__VA_ARGS__, decltype(nullptr)> = nullptr
