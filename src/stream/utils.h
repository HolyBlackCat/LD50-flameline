#pragma once

#include <iterator>
#include <type_traits>

namespace Stream
{
    namespace impl
    {
        // Checks if `T` is a container consisting of single-byte arithmetic objects.
        template <typename T> using detect_flat_byte_container = decltype(
            std::data(std::declval<const T&>()),
            std::size(std::declval<const T&>()),
            std::enable_if_t<
                std::is_arithmetic_v<std::remove_reference_t<decltype(*std::data(std::declval<const T&>()))>> &&
                sizeof(*std::data(std::declval<const T&>())) == 1
            >()
        );
    }
}
