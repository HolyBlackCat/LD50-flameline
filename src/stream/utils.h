#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace Stream
{
    enum class capacity_t : std::size_t {};

    enum ExceptionPrefixStyle : std::uint8_t
    {
        minimal          = 0,
        with_target_name = 1 << 0,
    };
    [[nodiscard]] inline ExceptionPrefixStyle operator&(ExceptionPrefixStyle a, ExceptionPrefixStyle b) {return ExceptionPrefixStyle(std::uint8_t(a) & std::uint8_t(b));}
    [[nodiscard]] inline ExceptionPrefixStyle operator|(ExceptionPrefixStyle a, ExceptionPrefixStyle b) {return ExceptionPrefixStyle(std::uint8_t(a) | std::uint8_t(b));}


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
