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
        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options) const override
        {
            if (!object)
            {
                output.WriteChar('?');
            }
            else
            {
                output.WriteChar(':');
                Interface<elem_t>().ToString(*object, output, options);
            }
        }

        void FromString(T &object, Stream::Input &input, const FromStringOptions &options) const override
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

            Interface<elem_t>().FromString(*object, input, options);
        }

        void ToBinary(const T &object, Stream::Output &output) const override
        {
            bool exists = object.has_value();
            Interface(exists).ToBinary(exists, output);
            if (exists)
                Interface<elem_t>().ToBinary(*object, output);
        }

        void FromBinary(T &object, Stream::Input &input, const FromBinaryOptions &options) const override
        {
            bool exists = 0;
            Interface(exists).FromBinary(exists, input, options);
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

            Interface<elem_t>().FromBinary(*object, input, options);
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
