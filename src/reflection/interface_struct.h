#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <exception>
#include <string_view>
#include <type_traits>
#include <utility>

#include "macros/generated.h"
#include "macros/named_macro_parameters.h"
#include "meta/misc.h"
#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "reflection/structs.h"
#include "strings/common.h"

namespace Refl
{
    namespace impl::Class
    {
        template <typename T, bool IsBase>
        constexpr bool ShouldSkipLow()
        {
            // By default - skip.
            bool skip = true;

            // If it's not a base class, and we don't know the members, then don't skip.
            // Bases with unknown members are skipped.
            if (!IsBase && !Refl::Class::members_known<T>)
                skip = false;

            // It at least one of the members is not skipped, then don't skip.
            Meta::cexpr_for<Refl::Class::member_count<T>>([&](auto index)
            {
                constexpr auto i = index.value;
                if (!ShouldSkipLow<Refl::Class::member_type<T, i>, false>())
                    skip = false;
            });

            // It at least one of the bases is not skipped, then don't skip.
            // If the class itself is a base class, don't consider virtual bases.
            using base_list = std::conditional_t<IsBase, Refl::Class::bases<T>, Refl::Class::combined_bases<T>>;
            Meta::cexpr_for<Meta::list_size<base_list>>([&](auto index)
            {
                constexpr auto i = index.value;
                if (!ShouldSkipLow<Meta::list_type_at<base_list, i>, true>())
                    skip = false;
            });

            return skip;
        }

        // Indicates if a specific base class should be skipped when [de]serializing.
        template <typename T> inline constexpr bool skip_base = ShouldSkipLow<T, true>();
        // Indicates if a specific field type should be skipped when [de]serializing.
        template <typename T> inline constexpr bool skip_member = ShouldSkipLow<T, false>();
    }

    template <typename T>
    class Interface_Struct : public InterfaceBasic<T>
    {
      public:
        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options, impl::ToStringState state) const override
        {
            static_assert(Class::members_known<T>, "Can't convert T to string: its members are not reflected.");

            constexpr bool named_members = Class::member_names_known<T>;

            output.WriteChar("({"[named_members]);

            auto next_member_state = state.MemberOrElem(options);
            auto next_base_state = state.BaseClass(options);

            // Force a single-line representation if member names aren't known and the struct has no members with 'non-short' representation.
            constexpr bool force_single_line = !Class::member_names_known<T> && []{
                bool ret = true;
                Meta::cexpr_for<Class::member_count<T>>([&](auto index)
                {
                    constexpr auto i = index.value;
                    if (!impl::HasShortStringRepresentation<Class::member_type<T, i>>::value)
                        ret = false;
                });

                Meta::cexpr_for<Meta::list_size<Class::combined_bases<T>>>([&](auto index)
                {
                    constexpr auto i = index.value;
                    if (!impl::HasShortStringRepresentation<Meta::list_type_at<Class::combined_bases<T>, i>>::value)
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
                        output.WriteChar('\n').WriteChar(' ', state.CurIndent() + options.indent);
                }
                else
                {
                    output.WriteChar(',');
                    if (options.pretty)
                    {
                        if (force_single_line)
                            output.WriteChar(' ');
                        else
                            output.WriteChar('\n').WriteChar(' ', state.CurIndent() + options.indent);
                    }
                }
            };

            // Writes a base class to the output.
            // `tag` is `Meta::tag<BaseClass>`.
            auto WriteBase = [&](auto tag)
            {
                using base_type = typename decltype(tag)::type;

                if constexpr (!impl::Class::skip_base<base_type>)
                {
                    WriteSeparator();

                    if constexpr (named_members)
                    {
                        static_assert(Class::name_known<base_type>, "Name of this base class is not known.");
                        output.WriteString(Class::name<base_type>);
                        if (options.pretty)
                            output.WriteChar(' ');
                    }

                    // We use a pointer cast instead of a reference one to catch cases where the derived class doesn't actually inherit from this base, but merely overloads the conversion operator.
                    const base_type &base_ref = *static_cast<const base_type *>(&object);
                    Refl::Interface(base_ref).ToString(base_ref, output, options, next_base_state); // A qualified call prevents unwanted ADL.
                }
            };

            // Output virtual bases.
            if (state.NeedVirtualBases())
            {
                using virt_bases = Class::virtual_bases<T>;
                Meta::cexpr_for<Meta::list_size<virt_bases>>([&](auto index)
                {
                    constexpr auto i = index.value;
                    WriteBase(Meta::tag<Meta::list_type_at<virt_bases, i>>{});
                });
            }

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

                if constexpr (!impl::Class::skip_member<type>)
                {
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

                    Refl::Interface(ref).ToString(ref, output, options, next_member_state); // A qualified call prevents unwanted ADL.
                }
            });

            // Output a trailing comma if we're using a multiline representation.
            if (!first && options.pretty && !force_single_line)
                output.WriteString(",\n").WriteChar(' ', state.CurIndent());

            output.WriteChar(")}"[named_members]);
        }

        void FromString(T &object, Stream::Input &input, const FromStringOptions &options, impl::FromStringState state) const override
        {
            static_assert(Class::members_known<T>, "Can't convert string to T: its members are not reflected.");

            constexpr bool named_members = Class::member_names_known<T>;

            auto next_member_state = state.MemberOrElem(options);
            auto next_base_state = state.BaseClass(options);

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
                    Utils::SkipWhitespaceAndComments(input);

                    // Stop on a closing brace. If this triggers, we either had 0 entries, or had a trailing comma.
                    if (input.Discard<Stream::if_present>('}'))
                        break;

                    // Get member or base name.
                    std::string name = input.Extract(Stream::Char::SeqIdentifier{});
                    Utils::SkipWhitespaceAndComments(input);

                    char first_char = input.PeekChar();

                    // Check if this looks like a member or like a base class.
                    if (first_char == '{' || first_char == '(')
                    {
                        // We got a base class.
                        std::size_t base_index = Class::CombinedBaseIndex<T>(name);
                        if (base_index == std::size_t(-1))
                            Program::Error(input.GetExceptionPrefix() + "Unknown base class: `" + name + "`.");

                        Meta::with_cexpr_value<combined_base_count>(base_index, [&](auto index)
                        {
                            constexpr auto i = index.value;
                            if (!state.NeedVirtualBases() && i >= Meta::list_size<Class::bases<T>>)
                                Program::Error(input.GetExceptionPrefix() + "Virtual base class `" + name + "` must be mentioned in the most derived class, not here.");

                            if (obtained_bases[i])
                                Program::Error(input.GetExceptionPrefix() + "Base class mentioned more than once: `" + name + "`.");

                            using this_base = Meta::list_type_at<combined_bases, i>;

                            if constexpr (impl::Class::skip_base<this_base>)
                            {
                                Program::Error(input.GetExceptionPrefix() + "Empty base class is mentioned: `" + name + "`.");
                            }
                            else
                            {
                                // We use a pointer cast instead of a reference one to catch cases where the derived class doesn't actually inherit from this base, but merely overloads the conversion operator.
                                auto &base_ref = *static_cast<this_base *>(&object);
                                Interface<this_base>().FromString(base_ref, input, options, next_base_state);

                                obtained_bases[i] = true;
                            }
                        });
                    }
                    else
                    {
                        // We got a member variable.
                        input.Discard('=');
                        Utils::SkipWhitespaceAndComments(input);

                        std::size_t member_index = Class::MemberIndex<T>(name);
                        if (member_index == std::size_t(-1))
                            Program::Error(input.GetExceptionPrefix() + "Unknown field: `" + name + "`.");

                        Meta::with_cexpr_value<Class::member_count<T>>(member_index, [&](auto index)
                        {
                            constexpr auto i = index.value;
                            if (obtained_members[i])
                                Program::Error(input.GetExceptionPrefix() + "Field mentioned more than once: `" + name + "`.");

                            if constexpr (impl::Class::skip_member<Class::member_type<T, i>>)
                            {
                                Program::Error(input.GetExceptionPrefix() + "Empty field is mentioned: `" + name + "`.");
                            }
                            else
                            {
                                auto &member_ref = Class::Member<i>(object);
                                Refl::Interface(member_ref).FromString(member_ref, input, options, next_member_state); // A qualified call prevents unwanted ADL.

                                obtained_members[i] = true;
                            }
                        });
                    }

                    // Skip whitespace after the entry.
                    Utils::SkipWhitespaceAndComments(input);

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
                        if constexpr (!Class::member_has_attrib<T, i, Optional> && !impl::Class::skip_member<Class::member_type<T, i>>)
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
                        if constexpr (!Class::class_has_attrib<this_base, Optional> && !impl::Class::skip_base<this_base>)
                        {
                            if (!state.NeedVirtualBases() && i >= Meta::list_size<Class::bases<T>>)
                                return;

                            if (!obtained_bases[i])
                                Program::Error(input.GetExceptionPrefix() + "Base class `" + Class::name<this_base> + "` is missing.");
                        }
                    });
                }
            }
            else
            {
                input.Discard('(');

                bool first = true;

                auto ReadEntry = [&](auto &ref, decltype(next_member_state) next_state)
                {
                    if (first)
                    {
                        first = false;
                        Utils::SkipWhitespaceAndComments(input);
                    }
                    else
                    {
                        Utils::SkipWhitespaceAndComments(input);
                        input.Discard(',');
                        Utils::SkipWhitespaceAndComments(input);
                    }

                    Refl::Interface(ref).FromString(ref, input, options, next_state); // A qualified call prevents unwanted ADL.
                };

                // Read virtual bases.
                if (state.NeedVirtualBases())
                {
                    using virt_bases = Class::virtual_bases<T>;
                    Meta::cexpr_for<Meta::list_size<virt_bases>>([&](auto index)
                    {
                        constexpr auto i = index.value;
                        using base_type = Meta::list_type_at<virt_bases, i>;
                        if constexpr (!impl::Class::skip_base<base_type>)
                            ReadEntry(*static_cast<base_type *>(&object), next_base_state);
                    });
                }

                // Read regular bases.
                using bases = Class::bases<T>;
                Meta::cexpr_for<Meta::list_size<bases>>([&](auto index)
                {
                    constexpr auto i = index.value;
                    using base_type = Meta::list_type_at<bases, i>;
                    if constexpr (!impl::Class::skip_base<base_type>)
                        ReadEntry(*static_cast<base_type *>(&object), next_base_state);
                });

                // Read members.
                Meta::cexpr_for<Class::member_count<T>>([&](auto index)
                {
                    constexpr auto i = index.value;
                    using type = const Class::member_type<T, i>;
                    if constexpr (!impl::Class::skip_member<type>)
                        ReadEntry(Class::Member<i>(object), next_member_state);
                });

                Utils::SkipWhitespaceAndComments(input);
                if (!first && input.Discard<Stream::if_present>(','))
                    Utils::SkipWhitespaceAndComments(input);
                input.Discard(')');
            }
        }

        void ToBinary(const T &object, Stream::Output &output, const ToBinaryOptions &options, impl::ToBinaryState state) const override
        {
            auto next_member_state = state.MemberOrElem(options);
            auto next_base_state = state.BaseClass(options);

            auto WriteEntry = [&](auto &ref, decltype(next_member_state) next_state)
            {
                Refl::Interface(ref).ToBinary(ref, output, options, next_state); // A qualified call prevents unwanted ADL.
            };

            // Write virtual bases.
            if (state.NeedVirtualBases())
            {
                using virt_bases = Class::virtual_bases<T>;
                Meta::cexpr_for<Meta::list_size<virt_bases>>([&](auto index)
                {
                    constexpr auto i = index.value;
                    using base_type = Meta::list_type_at<virt_bases, i>;
                    if constexpr (!impl::Class::skip_base<base_type>)
                        WriteEntry(*static_cast<const base_type *>(&object), next_base_state);
                });
            }

            // Write regular bases.
            using bases = Class::bases<T>;
            Meta::cexpr_for<Meta::list_size<bases>>([&](auto index)
            {
                constexpr auto i = index.value;
                using base_type = Meta::list_type_at<bases, i>;
                if constexpr (!impl::Class::skip_base<base_type>)
                    WriteEntry(*static_cast<const base_type *>(&object), next_base_state);
            });

            // Write members.
            Meta::cexpr_for<Class::member_count<T>>([&](auto index)
            {
                constexpr auto i = index.value;
                using type = const Class::member_type<T, i>;
                if constexpr (!impl::Class::skip_member<type>)
                    WriteEntry(Class::Member<i>(object), next_member_state);
            });
        }

        void FromBinary(T &object, Stream::Input &input, const FromBinaryOptions &options, impl::FromBinaryState state) const override
        {
            auto next_member_state = state.MemberOrElem(options);
            auto next_base_state = state.BaseClass(options);

            auto ReadEntry = [&](auto &ref, decltype(next_member_state) next_state)
            {
                Refl::Interface(ref).FromBinary(ref, input, options, next_state); // A qualified call prevents unwanted ADL.
            };

            // Write virtual bases.
            if (state.NeedVirtualBases())
            {
                using virt_bases = Class::virtual_bases<T>;
                Meta::cexpr_for<Meta::list_size<virt_bases>>([&](auto index)
                {
                    constexpr auto i = index.value;
                    using base_type = Meta::list_type_at<virt_bases, i>;
                    if constexpr (!impl::Class::skip_base<base_type>)
                        ReadEntry(*static_cast<base_type *>(&object), next_base_state);
                });
            }

            // Write regular bases.
            using bases = Class::bases<T>;
            Meta::cexpr_for<Meta::list_size<bases>>([&](auto index)
            {
                constexpr auto i = index.value;
                using base_type = Meta::list_type_at<bases, i>;
                if constexpr (!impl::Class::skip_base<base_type>)
                    ReadEntry(*static_cast<base_type *>(&object), next_base_state);
            });

            // Write members.
            Meta::cexpr_for<Class::member_count<T>>([&](auto index)
            {
                constexpr auto i = index.value;
                using type = const Class::member_type<T, i>;
                if constexpr (!impl::Class::skip_member<type>)
                    ReadEntry(Class::Member<i>(object), next_member_state);
            });
        }
    };

    template <typename T>
    struct impl::SelectInterface<T, std::enable_if_t<Class::members_known<T>>>
    {
        using type = Interface_Struct<T>;
    };

    template <typename T>
    struct impl::HasShortStringRepresentation<T, std::enable_if_t<Class::members_known<T>>>
    {
        static constexpr bool value = []{
            // If member names are known, representation is considered long.
            if constexpr (Refl::Class::member_names_known<T>)
            {
                return false;
            }
            else
            {
                // Otherwise, check if representation of all bases and members is short.
                bool value = true;

                // Check members.
                Meta::cexpr_for<Refl::Class::member_count<T>>([&](auto index)
                {
                    if (!impl::HasShortStringRepresentation<Refl::Class::member_type<T, index.value>>::value)
                        value = false;
                });

                // Check bases.
                using combined_bases = Refl::Class::combined_bases<T>;
                Meta::cexpr_for<Meta::list_size<combined_bases>>([&](auto index)
                {
                    if (!impl::HasShortStringRepresentation<Meta::list_type_at<combined_bases, index.value>>::value)
                        value = false;
                });

                return value;
            }
        }();
    };
}
