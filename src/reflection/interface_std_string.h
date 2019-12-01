#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <type_traits>

#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "utils/escape.h"
#include "utils/robust_math.h"

namespace Refl
{
    class Interface_StdString : public InterfaceBasic<std::string>
    {
      public:
        void ToString(const std::string &object, Stream::Output &output, const ToStringOptions &options = {}) const override
        {
            Strings::EscapeFlags flags = Strings::EscapeFlags::escape_double_quotes;
            if (options.multiline)
                flags = flags | Strings::EscapeFlags::multiline;

            output.WriteByte('"');
            Strings::Escape(object, output.GetOutputIterator(), flags);
            output.WriteByte('"');
        }

        void FromString(std::string &object, Stream::Input &input, const FromStringOptions &options = {}) override
        {
            (void)options;
            input.Discard<Stream::one>('"');
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
                object = Strings::Unescape(temp_str);
            }
            catch (std::exception &e)
            {
                Program::Error(input.GetExceptionPrefix() + e.what());
            }
        }

        using binary_length_t = std::uint32_t;
        static constexpr auto binary_length_byte_order = ByteOrder::little;

        // If the binary data ends up malformed, the size we read can be larger than the actual amount of bytes input contains.
        // This would cause us to allocate too much heap memory for no reason.
        // Rather than checking the remaining data size to make sure the size is valid, we simply refuse to `.reserve()` more bytes than a specific amount.
        static constexpr std::size_t max_reserved_size = 1024;

        void ToBinary(const std::string &object, Stream::Output &output) const override
        {
            if (Robust::not_representable_as<binary_length_t>(object.size()))
                Program::Error("The string is too long to be saved.");

            output.WriteWithByteOrder<binary_length_t>(binary_length_byte_order, object.size());
            output.WriteString(object);
        }

        void FromBinary(std::string &object, Stream::Input &input) override
        {
            binary_length_t len = input.ReadWithByteOrder<binary_length_t>(binary_length_byte_order);
            object = {};
            object.reserve(len < max_reserved_size ? len : max_reserved_size);
            while (len-- > 0)
                object += input.ReadChar();
        }
    };

    template <typename T>
    struct impl::SelectInterface<T, std::enable_if_t<std::is_same_v<T, std::string>>>
    {
        using type = Interface_StdString;
    };
}
