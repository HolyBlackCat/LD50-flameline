#pragma once

#include <exception>
#include <optional>
#include <type_traits>

#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "reflection/interface_scalar.h"

namespace Refl
{
    template <typename T>
    class Interface_StdOptional : public InterfaceBasic<T>
    {
        using elem_t = typename T::value_type;
      public:
        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options, impl::ToStringState state) const override
        {
            if (!object)
            {
                output.WriteChar('?');
            }
            else
            {
                output.WriteChar(':');
                Interface<elem_t>().ToString(*object, output, options, state.PartOfRepresentation(options));
            }
        }

        void FromString(T &object, Stream::Input &input, const FromStringOptions &options, impl::FromStringState state) const override
        {
            if (input.Discard<Stream::if_present>('?'))
            {
                object = {};
                return;
            }

            input.Discard(':');
            Utils::SkipWhitespaceAndComments(input);

            try
            {
                object = T(std::in_place);
            }
            catch (std::exception &e)
            {
                Program::Error(input.GetExceptionPrefix() + e.what());
            }

            Interface<elem_t>().FromString(*object, input, options, state.PartOfRepresentation(options));
        }

        void ToBinary(const T &object, Stream::Output &output, const ToBinaryOptions &options, impl::ToBinaryState state) const override
        {
            auto next_state = state.PartOfRepresentation(options);

            bool exists = object.has_value();
            Interface<bool>().ToBinary(exists, output, options, next_state);
            if (exists)
                Interface<elem_t>().ToBinary(*object, output, options, next_state);
        }

        void FromBinary(T &object, Stream::Input &input, const FromBinaryOptions &options, impl::FromBinaryState state) const override
        {
            auto next_state = state.PartOfRepresentation(options);

            bool exists = 0;
            Interface<bool>().FromBinary(exists, input, options, next_state);
            if (!exists)
            {
                object = {};
                return;
            }

            try
            {
                object = T(std::in_place);
            }
            catch (std::exception &e)
            {
                Program::Error(input.GetExceptionPrefix() + e.what());
            }

            Interface<elem_t>().FromBinary(*object, input, options, next_state);
        }
    };

    template <typename U>
    struct impl::SelectInterface<std::optional<U>>
    {
        using type = Interface_StdOptional<std::optional<U>>;
    };

    template <typename U>
    struct impl::HasShortStringRepresentation<std::optional<U>> : impl::HasShortStringRepresentation<U> {};
}
