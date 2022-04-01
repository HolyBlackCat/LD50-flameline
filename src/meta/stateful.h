#pragma once

#include <cstddef>
#include <type_traits>

#include "meta/common.h"
#include "meta/lists.h"

namespace Meta::Stateful
{
    namespace List
    {
        namespace impl
        {
            template <typename Name, std::size_t Index>
            struct ElemReader
            {
                friend constexpr auto adl_ImpListElem(ElemReader<Name, Index>);
            };

            template <typename Name, std::size_t Index, typename Value>
            struct ElemWriter
            {
                friend constexpr auto adl_ImpListElem(ElemReader<Name, Index>)
                {
                    return tag<Value>{};
                }
            };

            constexpr void adl_ImpListElem() {} // A dummy ADL target.

            template <typename Name, std::size_t Index, typename Unique>
            struct CalcSize : std::integral_constant<std::size_t, Index> {};

            template <typename Name, std::size_t Index, typename Unique>
            requires requires {typename decltype(adl_ImpListElem(ElemReader<Name, Index>{}))::type;}
            struct CalcSize<Name, Index, Unique> : CalcSize<Name, Index + 1, Unique> {};

            template <typename Name, std::size_t Index, typename Unique>
            using ReadElem = typename decltype(adl_ImpListElem(ElemReader<Name, Index>{}))::type;

            template <typename Name, typename I, typename Unique>
            struct ReadElemList {};
            template <typename Name, std::size_t ...I, typename Unique>
            struct ReadElemList<Name, std::index_sequence<I...>, Unique> {using type = type_list<ReadElem<Name, I, Unique>...>;};
        }

        struct DefaultUnique {};

        // Calculates the current list size.
        template <typename Name, typename Unique = DefaultUnique>
        inline constexpr std::size_t size = impl::CalcSize<Name, 0, Unique>::value;

        // Touch this type to append `Value` to the list.
        template <typename Name, typename Value, typename Unique = Value>
        using PushBack = impl::ElemWriter<Name, size<Name, Unique>, Value>;

        // Returns the type previously passed to `WriteState`, or causes a SFINAE error.
        template <typename Name, std::size_t I, typename Unique = DefaultUnique>
        using Elem = impl::ReadElem<Name, I, Unique>;

        template <typename Name, typename Unique = DefaultUnique>
        using Elems = typename impl::ReadElemList<Name, std::make_index_sequence<size<Name, Unique>>, Unique>::type;
    }
}
