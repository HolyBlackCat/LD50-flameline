#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <exception>
#include <string_view>
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

            // Force a single-line representation if member names aren't known and the struct has no members with 'non-short' representation.
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

            // Call this before outputting a field or a base.
            // Writes a comma, unless it's the first entry. Also adds some line-feeds and indents if necessary.
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

            // Writes a base class to the output.
            // `tag` is `Meta::tag<BaseClass>`.
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

                // We use a pointer cast instead of a reference one to catch cases where the derived class doesn't actually inherit from this base, but merely overloads the conversion operator.
                base_type &base_ref = *static_cast<base_type *>(&object);
                Interface(base_ref).ToString(base_ref, output, next_options);
            };

            // Output virtual bases.
            using virt_bases = Class::virtual_bases<T>;
            Meta::cexpr_for<Meta::list_size<virt_bases>>([&](auto index)
            {
                constexpr auto i = index.value;
                WriteBase(Meta::tag<Meta::list_type_at<virt_bases, i>>{});
            });

            // Output regular bases.
            using bases = Class::bases<T>;
            Meta::cexpr_for<Meta::list_size<bases>>([&](auto index)
            {
                constexpr auto i = index.value;
                WriteBase(Meta::tag<Meta::list_type_at<bases, i>>{});
            });

            // Output members.
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

            // Output a trailing comma if we're using a multiline representation.
            if (!first && options.pretty && !force_single_line)
                output.WriteString(",\n").WriteChar(' ', options.extra_indent);

            output.WriteChar(")}"[named_members]);
        }

        void FromString(T &object, Stream::Input &input, const FromStringOptions &options) const override
        {
            static_assert(Class::members_known<T>, "Can't convert string to T: its members are not reflected.");

            constexpr bool named_members = Class::member_names_known<T>;

            if constexpr (named_members)
            {
                // A list of all base class. First regular ones, then virtual ones.
                using combined_bases = Class::combined_bases<T>;
                constexpr std::size_t combined_base_count = Meta::list_size<combined_bases>;

                // Those flags indicate if a member/base was already deserialized.
                // Regular arrays can't be used here, since those arrays can be empty.
                std::array<bool, Class::member_count<T>> obtained_members = {};
                std::array<bool, combined_base_count> obtained_bases = {};

                input.Discard('{');

                while (true)
                {
                    // Skip whitespace before the entry.
                    Parsing::SkipWhitespaceAndComments(input);

                    // Stop on a closing brace. If this triggers, we either had 0 entries, or had a trailing comma.
                    if (input.Discard<Stream::if_present>('}'))
                        break;

                    // Get member or base name.
                    std::string name = input.Extract(Stream::Char::SeqIdentifier{});
                    Parsing::SkipWhitespaceAndComments(input);

                    char first_char = input.PeekChar();

                    // Check if this looks like a member or like a base class.
                    if (first_char == '{' || first_char == '(')
                    {
                        std::size_t base_index = Class::CombinedBaseIndex<T>(name);
                        if (base_index == std::size_t(-1))
                            Program::Error(input.GetExceptionPrefix() + "Unknown base class: `" + name + "`.");

                        Meta::with_cexpr_value<combined_base_count>(base_index, [&](auto index)
                        {
                            constexpr auto i = index.value;
                            if (obtained_bases[i])
                                Program::Error(input.GetExceptionPrefix() + "Base class mentioned more than once: `" + name + "`.");

                            using this_base = Meta::list_type_at<combined_bases, i>;

                            // We use a pointer cast instead of a reference one to catch cases where the derived class doesn't actually inherit from this base, but merely overloads the conversion operator.
                            auto &base_ref = *static_cast<this_base *>(&object);
                            Interface<this_base>().FromString(base_ref, input, options);

                            obtained_bases[i] = true;
                        });
                    }
                    else
                    {
                        // We got a member variable.
                        input.Discard('=');
                        Parsing::SkipWhitespaceAndComments(input);

                        std::size_t member_index = Class::MemberIndex<T>(name);
                        if (member_index == std::size_t(-1))
                            Program::Error(input.GetExceptionPrefix() + "Unknown field: `" + name + "`.");

                        Meta::with_cexpr_value<Class::member_count<T>>(member_index, [&](auto index)
                        {
                            constexpr auto i = index.value;
                            if (obtained_members[i])
                                Program::Error(input.GetExceptionPrefix() + "Field mentioned more than once: `" + name + "`.");

                            auto &member_ref = Class::Member<i>(object);
                            Interface(member_ref).FromString(member_ref, input, options);

                            obtained_members[i] = true;
                        });
                    }

                    // Skip whitespace after the entry.
                    Parsing::SkipWhitespaceAndComments(input);

                    // Skip comma after the entry.
                    if (!input.Discard<Stream::if_present>(','))
                    {
                        // No comma found, expect a closing brace.
                        input.Discard('}');
                        break;
                    }
                }

                // Make sure we got all required fields and bases.
                if (!options.ignore_missing_fields)
                {
                    // Check fields.
                    Meta::cexpr_for<Class::member_count<T>>([&](auto index)
                    {
                        constexpr auto i = index.value;
                        if constexpr (!Class::member_has_attrib<T, i, Optional>)
                        {
                            if (!obtained_members[i])
                                Program::Error(input.GetExceptionPrefix() + "Field `" + Class::MemberName<T>(i) + "` is missing.");
                        }
                    });

                    // Check bases.
                    Meta::cexpr_for<combined_base_count>([&](auto index)
                    {
                        constexpr auto i = index.value;
                        using this_base = Meta::list_type_at<combined_bases, i>;
                        if constexpr (!Class::class_has_attrib<this_base, Optional>)
                        {
                            if (!obtained_bases[i])
                                Program::Error(input.GetExceptionPrefix() + "Base class `" + Class::name<this_base> + "` is missing.");
                        }
                    });
                }
            }
        }

        void ToBinary(const T &object, Stream::Output &output) const override
        {
            (void)object;
            (void)output;
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
            (void)object;
            (void)input;
            (void)options;
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
