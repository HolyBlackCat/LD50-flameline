#pragma once

#include <array>
#include <cstdint>
#include <exception>
#include <limits>
#include <type_traits>
#include <variant>

#include "meta/misc.h"
#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "reflection/interface_scalar.h"
#include "reflection/structs.h"
#include "utils/robust_math.h"

namespace Refl
{
    namespace impl
    {
        using variant_index_binary_t = std::uint8_t; // Keep this unsigned, or adjust the validation logic below.
        inline constexpr auto variant_index_byte_order = ByteOrder::little;
    }

    template <typename T>
    class Interface_StdVariant : public InterfaceBasic<T>
    {
        static_assert(Robust::less_eq(std::variant_size_v<T>, std::numeric_limits<impl::variant_index_binary_t>::max()), "The variant is too large.");

        static constexpr std::array<const char *, std::variant_size_v<T>> ElemNames()
        {
            if constexpr (std::variant_size_v<T> == 0)
            {
                return {};
            }
            else
            {
                return Meta::cexpr_generate_array<std::variant_size_v<T>>([](auto index)
                {
                    return Class::name<std::variant_alternative_t<index.value, T>>;
                });
            }
        }

      public:
        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options, impl::ToStringState state) const override
        {
            if (object.valueless_by_exception())
                Program::Error(output.GetExceptionPrefix() + "Unable to serialize variant: Valueless by exception.");

            Meta::with_cexpr_value<std::variant_size_v<T>>(object.index(), [&](auto index)
            {
                constexpr auto i = index.value;
                using this_type = std::variant_alternative_t<i, T>;
                static_assert(Class::name_known<this_type>, "Name of this base class is not known.");
                output.WriteString(Class::name<this_type>);
                if (options.pretty)
                    output.WriteChar(' ');
                Interface<this_type>().ToString(std::get<i>(object), output, options, state.PartOfRepresentation(options));
            });
        }

        void FromString(T &object, Stream::Input &input, const FromStringOptions &options, impl::FromStringState state) const override
        {
            std::string name = input.Extract(Stream::Char::SeqIdentifier{});
            std::size_t index = Utils::GetStringIndex<ElemNames>(name.c_str());
            if (index == std::size_t(-1))
                Program::Error(input.GetExceptionPrefix() + "Unknown variant alternative name: `" + name + "`.");

            Utils::SkipWhitespaceAndComments(input);

            Meta::with_cexpr_value<std::variant_size_v<T>>(index, [&](auto index)
            {
                constexpr auto i = index.value;
                using this_type = std::variant_alternative_t<i, T>;
                this_type *ptr = nullptr;

                try
                {
                    ptr = &object.template emplace<i>();
                }
                catch (std::exception &e)
                {
                    Program::Error(input.GetExceptionPrefix() + e.what());
                }

                Interface<this_type>().FromString(*ptr, input, options, state.PartOfRepresentation(options));
            });
        }

        void ToBinary(const T &object, Stream::Output &output, const ToBinaryOptions &options, impl::ToBinaryState state) const override
        {
            if (object.valueless_by_exception())
                Program::Error(output.GetExceptionPrefix() + "Unable to serialize variant: Valueless by exception.");

            impl::variant_index_binary_t index = object.index(); // No range validation is necessary, since we have a static_assert.
            output.WriteWithByteOrder<impl::variant_index_binary_t>(impl::variant_index_byte_order, index);

            Meta::with_cexpr_value<std::variant_size_v<T>>(index, [&](auto index)
            {
                constexpr auto i = index.value;
                using this_type = std::variant_alternative_t<i, T>;
                Interface<this_type>().ToBinary(std::get<i>(object), output, options, state.PartOfRepresentation(options));
            });
        }

        void FromBinary(T &object, Stream::Input &input, const FromBinaryOptions &options, impl::FromBinaryState state) const override
        {
            auto index = input.ReadWithByteOrder<impl::variant_index_binary_t>(impl::variant_index_byte_order);
            if (Robust::greater_eq(index, std::variant_size_v<T>))
                Program::Error(input.GetExceptionPrefix() + "Variant alternative index is too large.");

            Meta::with_cexpr_value<std::variant_size_v<T>>(index, [&](auto index)
            {
                constexpr auto i = index.value;
                using this_type = std::variant_alternative_t<i, T>;
                this_type *ptr = nullptr;

                try
                {
                    ptr = &object.template emplace<i>();
                }
                catch (std::exception &e)
                {
                    Program::Error(input.GetExceptionPrefix() + e.what());
                }

                Interface<this_type>().FromBinary(*ptr, input, options, state.PartOfRepresentation(options));
            });
        }
    };

    template <typename ...P>
    struct impl::SelectInterface<std::variant<P...>>
    {
        using type = Interface_StdVariant<std::variant<P...>>;
    };

    template <typename ...P>
    struct impl::HasShortStringRepresentation<std::variant<P...>> : std::bool_constant<(impl::HasShortStringRepresentation<P>::value && ...)> {};
}
