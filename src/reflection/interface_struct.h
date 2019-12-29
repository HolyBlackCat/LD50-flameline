#pragma once

#include <cctype>
#include <cstddef>
#include <exception>
#include <type_traits>
#include <utility>

#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "reflection/structs.h"
#include "utils/macro.h"
#include "utils/meta.h"
#include "utils/named_macro_parameters.h"
#include "utils/strings.h"

namespace Refl
{
    template <typename T>
    class Interface_Struct : public InterfaceBasic<T>
    {
      public:
        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options) const override
        {
            static_assert(Class::members_known<T>, "Can't convert T to string: its members are not reflected.");

            constexpr bool named_members = Class::member_names_known<T>;

            output.WriteChar("({"[named_members]);

            auto next_options = options;
            if (options.pretty)
                next_options.extra_indent += options.indent;

            constexpr bool force_single_line = !Class::member_names_known<T> && []{
                bool ret = true;
                Meta::cexpr_for<Class::member_count<T>>([&](auto index)
                {
                    constexpr auto i = index.value;
                    if (!impl::HasShortStringRepresentation<Class::member_type<T, i>>::value)
                        ret = false;
                });
                return ret;
            }();

            bool first = true;

            auto WriteSeparator = [&]
            {
                if (first)
                {
                    first = false;
                    if (options.pretty && !force_single_line)
                        output.WriteChar('\n').WriteChar(' ', next_options.extra_indent);
                }
                else
                {
                    output.WriteChar(',');
                    if (options.pretty)
                    {
                        if (force_single_line)
                            output.WriteChar(' ');
                        else
                            output.WriteChar('\n').WriteChar(' ', next_options.extra_indent);
                    }
                }
            };

            auto WriteBase = [&](auto tag)
            {
                using base_type = const typename decltype(tag)::type;

                constexpr bool skip_this_base = Class::member_count<base_type> == 0 && Class::class_has_attrib<base_type, Refl::Optional>;
                if (skip_this_base)
                    return;

                WriteSeparator();

                if constexpr (named_members)
                {
                    static_assert(Class::name_known<base_type>, "Name of this base class is not known.");
                    output.WriteString(Class::name<base_type>);
                    if (options.pretty)
                        output.WriteChar(' ');
                }

                base_type &base_ref = *static_cast<base_type *>(&object); // Use a pointer cast rather than a reference cast to avoid any custom conversion operators.
                Interface(base_ref).ToString(base_ref, output, next_options);
            };

            using virt_bases = Class::recursive_virtual_bases<T>;
            Meta::cexpr_for<Meta::list_size<virt_bases>>([&](auto index)
            {
                constexpr auto i = index.value;
                WriteBase(Meta::tag<Meta::list_type_at<virt_bases, i>>{});
            });

            using bases = Class::bases<T>;
            Meta::cexpr_for<Meta::list_size<bases>>([&](auto index)
            {
                constexpr auto i = index.value;
                WriteBase(Meta::tag<Meta::list_type_at<bases, i>>{});
            });

            Meta::cexpr_for<Class::member_count<T>>([&](auto index)
            {
                constexpr auto i = index.value;
                using type = const Class::member_type<T, i>;
                type &ref = Class::Member<i>(object);

                WriteSeparator();
                if constexpr (named_members)
                {
                    output.WriteString(Class::MemberName<T>(i));
                    if (options.pretty)
                        output.WriteString(" = ");
                    else
                        output.WriteChar('=');
                }

                Interface(ref).ToString(ref, output, next_options);
            });

            if (!first && options.pretty && !force_single_line)
                output.WriteString(",\n").WriteChar(' ', options.extra_indent);

            output.WriteChar(")}"[named_members]);
        }

        void FromString(T &object, Stream::Input &input, const FromStringOptions &options) const override
        {
            // Clear(object);

            // input.Discard('[');

            // while (true)
            // {
            //     Parsing::SkipWhitespaceAndComments(input);
            //     if (input.Discard<Stream::if_present>(']'))
            //         break;

            //     mutable_elem_t elem{};
            //     Interface<mutable_elem_t>().FromString(elem, input, options);

            //     try
            //     {
            //         PushBack(object, std::move(elem));
            //     }
            //     catch (std::exception &e)
            //     {
            //         Program::Error(input.GetExceptionPrefix() + e.what());
            //     }

            //     Parsing::SkipWhitespaceAndComments(input);

            //     if (!input.Discard<Stream::if_present>(','))
            //     {
            //         input.Discard(']');
            //         break;
            //     }
            // }
        }

        void ToBinary(const T &object, Stream::Output &output) const override
        {
            // impl::container_length_binary_t len;
            // if (Robust::conversion_fails(object.size(), len))
            //     Program::Error(output.GetExceptionPrefix() + "The container is too long.");
            // output.WriteWithByteOrder<impl::container_length_binary_t>(impl::container_length_byte_order, len);

            // ForEach(object, [&](const elem_t &elem)
            // {
            //     Interface<elem_t>().ToBinary(elem, output);
            // });
        }

        void FromBinary(T &object, Stream::Input &input, const FromBinaryOptions &options) const override
        {
            // std::size_t len;
            // if (Robust::conversion_fails(input.ReadWithByteOrder<impl::container_length_binary_t>(impl::container_length_byte_order), len))
            //     Program::Error(input.GetExceptionPrefix() + "The string is too long.");

            // std::size_t max_reserved_elems = options.max_reserved_size / sizeof(elem_t);

            // Clear(object);
            // Reserve(object, len < max_reserved_elems ? len : max_reserved_elems);

            // while (len-- > 0)
            // {
            //     mutable_elem_t elem{};
            //     Interface<mutable_elem_t>().FromBinary(elem, input, options);

            //     try
            //     {
            //         PushBack(object, std::move(elem));
            //     }
            //     catch (std::exception &e)
            //     {
            //         Program::Error(input.GetExceptionPrefix() + e.what());
            //     }
            // }
        }
    };

    template <typename T>
    struct impl::SelectInterface<T, std::enable_if_t<Class::members_known<T>>>
    {
        using type = Interface_Struct<T>;
    };
}
