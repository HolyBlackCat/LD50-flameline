#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <type_traits>

#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "reflection/interface_container.h"
#include "strings/escape.h"
#include "utils/robust_math.h"

namespace Refl
{
    class Interface_StdString : public InterfaceBasic<std::string>
    {
      public:
        void ToString(const std::string &object, Stream::Output &output, const ToStringOptions &options, impl::ToStringState state) const override
        {
            (void)state;

            Strings::EscapeFlags flags = Strings::EscapeFlags::escape_double_quotes;
            if (options.multiline_strings)
                flags = flags | Strings::EscapeFlags::multiline;

            output.WriteByte('"');
            Strings::Escape(object, output.GetOutputIterator(), flags);
            output.WriteByte('"');
        }

        void FromString(std::string &object, Stream::Input &input, const FromStringOptions &options, impl::FromStringState state) const override
        {
            (void)options;
            (void)state;

            input.Discard('"');
            std::string temp_str;
            while (true)
            {
                char ch = input.ReadChar();
                if (ch == '"')
                    break;

                temp_str += ch;
                if (ch == '\\')
                    temp_str += input.ReadChar();
            }

            try
            {
                object = Strings::Unescape(temp_str, Strings::UnescapeFlags::strip_cr_bytes);
            }
            catch (std::exception &e)
            {
                Program::Error(input.GetExceptionPrefix() + e.what());
            }
        }

        void ToBinary(const std::string &object, Stream::Output &output, const ToBinaryOptions &options, impl::ToBinaryState state) const override
        {
            (void)options;
            (void)state;

            impl::container_length_binary_t len;
            if (Robust::conversion_fails(object.size(), len))
                Program::Error(output.GetExceptionPrefix() + "The string is too long.");

            output.WriteWithByteOrder<impl::container_length_binary_t>(impl::container_length_byte_order, len);
            output.WriteString(object);
        }

        void FromBinary(std::string &object, Stream::Input &input, const FromBinaryOptions &options, impl::FromBinaryState state) const override
        {
            (void)state;

            std::size_t len;
            if (Robust::conversion_fails(input.ReadWithByteOrder<impl::container_length_binary_t>(impl::container_length_byte_order), len))
                Program::Error(input.GetExceptionPrefix() + "The string is too long.");

            object = {};
            object.reserve(len < options.max_reserved_size ? len : options.max_reserved_size);
            while (len-- > 0)
                object += input.ReadChar();
        }
    };

    template <typename T>
    struct impl::SelectInterface<T, std::enable_if_t<std::is_same_v<T, std::string>>>
    {
        using type = Interface_StdString;
    };

    template <>
    struct impl::ForceNotContainer<std::string> : std::true_type {};
}
