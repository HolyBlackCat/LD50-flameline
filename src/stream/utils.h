#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace Stream
{
    enum class capacity_t : std::size_t {};

    enum ExceptionPrefixStyle : std::uint8_t
    {
        no_prefix        = 0,
        with_target_name = 1 << 0,
        with_location    = 1 << 1, // The output stream doesn't support this flag.
    };
    [[nodiscard]] inline ExceptionPrefixStyle operator&(ExceptionPrefixStyle a, ExceptionPrefixStyle b) {return ExceptionPrefixStyle(std::uint8_t(a) & std::uint8_t(b));}
    [[nodiscard]] inline ExceptionPrefixStyle operator|(ExceptionPrefixStyle a, ExceptionPrefixStyle b) {return ExceptionPrefixStyle(std::uint8_t(a) | std::uint8_t(b));}


    namespace impl
    {
        // Checks if `T` is a container consisting of single-byte arithmetic objects.
        template <typename T>
        concept FlatByteContainer = requires(const T t)
        {
            std::data(t);
            std::size(t);
            requires std::is_arithmetic_v<std::remove_reference_t<decltype(*std::data(t))>>;
            requires sizeof(*std::data(t)) == 1;
        };
    }
}
