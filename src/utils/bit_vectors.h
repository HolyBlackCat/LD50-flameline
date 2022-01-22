#pragma once

#include <climits>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>


// Functions for accessing ranges of unsigned integers as bits.

namespace BitVec
{
    // A container of bits, possibly const.
    // Must have random-access iterators and unsigned integral elements.
    template <typename T>
    concept Container = requires(const T &t, std::remove_cvref_t<decltype(*std::begin(t))> elem)
    {
        requires !std::is_reference_v<T>;
        requires std::unsigned_integral<decltype(elem)>;
        requires !std::is_volatile_v<decltype(elem)>;
        { std::size(t) } -> std::same_as<std::size_t>;
        requires std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<decltype(std::begin(t))>::iterator_category>;
    };

    // The element type of a bit container, possibly const.
    template <Container T>
    using ContainerElem = std::remove_reference_t<decltype(*std::begin(std::declval<T &>()))>;

    // Same as `MutableContainer<T>`, but requires non-const elements.
    template <typename T>
    concept MutableContainer = Container<T> && !std::is_const_v<ContainerElem<T>>;

    // A resizable container with non-const elements.
    template <typename T>
    concept ResizableContainer = MutableContainer<T> && requires(T &t)
    {
        t.resize(0zu);
    };


    // Given a type (element of a suitable `Container`), returns it's bit size.
    template <std::unsigned_integral T>
    inline constexpr std::size_t bit_width = sizeof(T) * CHAR_BIT;


    // Returns the number of bits in a container.
    template <Container T>
    [[nodiscard]] constexpr std::size_t Size(const T &c)
    {
        return std::size(c) * bit_width<ContainerElem<T>>;
    }

    template <std::unsigned_integral T>
    struct BitLoc
    {
        std::size_t index = 0;
        std::remove_const_t<T> mask = 0;
    };

    // Given a bit index, the index of the integer storing the bit, and a mask for that bit.
    template <std::unsigned_integral T>
    [[nodiscard]] constexpr BitLoc<T> BitLocation(std::size_t i)
    {
        return {i / bit_width<T>, T(T(1) << i % bit_width<T>)};
    }


    // Reads a bit by its index, or returns 0 if out of range.
    template <Container T>
    [[nodiscard]] constexpr bool GetBitOrZero(const T &c, std::size_t i)
    {
        if (auto loc = (BitLocation<ContainerElem<T>>)(i); loc.index < std::size(c))
            return bool(std::begin(c)[loc.index] & loc.mask);
        else
            return false;
    }

    // Reads a bit by its index, or throws if out of range.
    template <Container T>
    [[nodiscard]] constexpr bool GetBitOrThrow(const T &c, std::size_t i)
    {
        if (auto loc = (BitLocation<ContainerElem<T>>)(i); loc.index < std::size(c))
            return bool(std::begin(c)[loc.index] & loc.mask);
        else
            throw std::runtime_error("Bit index is out of range.");
    }


    enum class Op {zero, one, toggle}; // Do not reorder.
    using enum Op;

    // A helper class, serving as function parameter.
    struct OpParam
    {
        Op op = one;

        constexpr OpParam(Op op) : op(op) {}
        constexpr OpParam(bool value) : op(Op(value)) {}

        // Applies the operation to the specified bit. UB if the index is out of range.
        template <MutableContainer T>
        constexpr void Apply(T &c, BitLoc<ContainerElem<T>> loc)
        {
            ContainerElem<T> &num = std::begin(c)[loc.index];

            switch (op)
            {
              case zero:
                num &= ~loc.mask;
                break;
              case one:
                num |= loc.mask;
                break;
              case toggle:
                if (num & loc.mask)
                    num &= ~loc.mask;
                else
                    num |= loc.mask;
                break;
            }
        }
    };

    // Modifies bit `i` in `c` (sets to 1 by default), or throws if out of range.
    template <MutableContainer T>
    constexpr void SetBitOrThrow(T &c, std::size_t i, OpParam op = true)
    {
        if (auto loc = (BitLocation<ContainerElem<T>>)(i); loc.index < std::size(c))
            op.Apply(c, loc);
        else
            throw std::runtime_error("Bit index is out of range.");
    }

    // Modifies bit `i` in `c` (sets to 1 by default), possibly resizing the container if it's too small.
    template <MutableContainer T>
    constexpr void SetBitAutoResize(T &c, std::size_t i, OpParam op = true)
    {
        auto loc = (BitLocation<ContainerElem<T>>)(i);
        if (loc.index >= std::size(c))
            c.resize(loc.index + 1);
        op.Apply(c, loc);
    }
}
