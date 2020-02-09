#pragma once

#include <exception>
#include <type_traits>

#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "strings/lexical_cast.h"

namespace Refl
{
    namespace impl
    {
        inline constexpr auto scalar_byte_order = ByteOrder::little;
    }

    template <typename T>
    class Interface_Scalar : public InterfaceBasic<T>
    {
      public:
        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options, impl::ToStringState state) const override
        {
            (void)options;
            (void)state;

            char buf[Strings::ToStringMaxBufferLen()];
            Strings::ToString(buf, sizeof buf, object);
            output.WriteString(buf);
        }

        void FromString(T &object, Stream::Input &input, const FromStringOptions &options, impl::FromStringState state) const override
        {
            (void)options;
            (void)state;

            constexpr bool is_fp = std::is_floating_point_v<T>;

            auto category = Stream::Char::Is(is_fp ? "a real number" : "an integer", [](char ch)
            {
                bool ok = Stream::Char::IsAlphaOrDigit{}(ch) || ch == '+' || ch == '-' || ch == Strings::CharDigitSeparator();
                if constexpr (is_fp)
                    ok = ok || ch == '.' || ch == Strings::CharLongDoublePartsSeparator();
                return ok;
            });

            std::string str = input.Extract(category);
            try
            {
                object = Strings::FromString<T>(str);
            }
            catch (std::exception &e)
            {
                Program::Error(input.GetExceptionPrefix() + e.what());
            }
        }

        void ToBinary(const T &object, Stream::Output &output, const ToBinaryOptions &options, impl::ToBinaryState state) const override
        {
            (void)options;
            (void)state;

            output.WriteWithByteOrder<T>(impl::scalar_byte_order, object);
        }

        void FromBinary(T &object, Stream::Input &input, const FromBinaryOptions &options, impl::FromBinaryState state) const override
        {
            (void)options;
            (void)state;

            object = input.ReadWithByteOrder<T>(impl::scalar_byte_order);
        }
    };

    template <typename T>
    struct impl::SelectInterface<T, std::enable_if_t<std::is_arithmetic_v<T>>>
    {
        using type = Interface_Scalar<T>;
    };

    template <typename T>
    struct impl::HasShortStringRepresentation<T, std::enable_if_t<std::is_arithmetic_v<T>>> : std::true_type {};
}
