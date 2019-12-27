#pragma once

#include <cctype>
#include <cstddef>
#include <cstring>
#include <exception>
#include <functional>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "reflection/interface_std_string.h"
#include "utils/abi.h"
#include "utils/macro.h"
#include "utils/meta.h"
#include "utils/named_macro_parameters.h"
#include "utils/strings.h"

namespace Refl
{
    // A base class for field attributes.
    struct BasicAttribute {};

    namespace impl::Class
    {
        // Stores the list of attributes for a single `REFL_DECL` declaration.
        // `N` is the amount of variables, `T` is a `Meta::type_list` of attributes.
        template <std::size_t N, typename T> struct Attr {};
        template <std::size_t N, typename ...P> struct Attr<N, Meta::type_list<P...>>
        {
            static_assert(((std::is_base_of_v<BasicAttribute, P> && !std::is_same_v<BasicAttribute, P>) && ...), "Attributes must inherit from Refl::BasicAttribute.");
            using type = Meta::type_list<P...>;
        };

        // Obtains a pretty type name.
        // Names of standard types might be not portable, so if the type name mentions `std::`, a debug assertion is triggered.
        template <typename T> const std::string &TypeName()
        {
            static std::string ret = []{
                std::string ret = Strings::Condense(Abi::Demangle{}(typeid(T).name()));
                DebugAssert("Attempt to get the name of a standard type:\n" + ret, [&]{
                    for (char &ch : ret)
                    {
                        if (!std::isalpha((unsigned char)ch))
                        {
                            if (std::strncmp(&ch+1, "std::", 5) == 0)
                                return false;
                        }
                    }
                    return true;
                }());
                return ret;
            }();
            return ret;
        }
    }

    namespace Class
    {
        namespace Macro
        {
            // Those can be used to obtain macro-generated class metadata.
            // Here, `T` is never const.

            namespace impl
            {
                // Obtains the internal (member) metadata type for T, one of the two possible metadata types.
                // Should be SFINAE-friendly.
                template <typename T, typename = void> struct member_metadata {};
                template <typename T> struct member_metadata<T, Meta::void_type<decltype(std::declval<const T &>().zrefl_MembersHelper())>>
                {
                    using type = decltype(std::declval<const T &>().zrefl_MembersHelper());
                };
                template <typename T> using member_metadata_t = typename member_metadata<T>::type;

                void zrefl_StructFunc(); // Dummy ADL target.

                // Obtains the external (non-member) metadata type for T, one of the two possible metadata types.
                // Should be SFINAE-friendly.
                template <typename T, typename = void> struct nonmember_metadata {};
                template <typename T> struct nonmember_metadata<T, Meta::void_type<decltype(zrefl_StructFunc(Meta::tag<T>{}))>>
                {
                    using type = decltype(zrefl_StructFunc(Meta::tag<T>{}));
                };
                template <typename T> using nonmember_metadata_t = typename nonmember_metadata<T>::type;

                // Applies `Alias` to the metadata of `T`.
                // Both member and non-member metadata types are examined.
                template <template <typename> typename Alias, typename T, typename = void>
                struct metadata_type {};

                template <template <typename> typename Alias, typename T>
                struct metadata_type<Alias, T, Meta::void_type<Alias<member_metadata_t<T>>>>
                {
                    using type = Alias<member_metadata_t<T>>;
                };

                template <template <typename> typename Alias, typename T>
                struct metadata_type<Alias, T, Meta::void_type<Alias<nonmember_metadata_t<T>>>>
                {
                    using type = Alias<nonmember_metadata_t<T>>;
                };

                template <template <typename> typename Alias, typename T>
                using metadata_type_t = typename metadata_type<Alias, T>::type;

                // Aliases for `metadata_type`.
                template <typename T> using type_bases = typename T::bases;
                template <typename T> using type_virt_bases = typename T::virt_bases;
                template <typename T> using type_member_ptrs = typename T::member_ptrs;
                template <typename T> using type_member_attribs = typename T::member_attribs;
                template <typename T> using value_member_names_func = Meta::value_tag<T::member_name>;
            }

            // All of those should be SFINAE-friendly.
            // Returns a `Meta::type_list` of bases if they are known.
            template <typename T> using bases = impl::metadata_type_t<impl::type_bases, T>;
            // Returns a `Meta::type_list` of virtual bases if they are known.
            template <typename T> using virt_bases = impl::metadata_type_t<impl::type_virt_bases, T>;
            // Returns a `Meta::value_list` of member pointers.
            template <typename T> using member_ptrs = impl::metadata_type_t<impl::type_member_ptrs, T>;
            // Returns a `Meta::type_list` of `Refl::impl::Class::Attr`, one per `REFL_DECL`.
            template <typename T> using member_attribs = impl::metadata_type_t<impl::type_member_attribs, T>;
            // `::value` is a function pointer `const char (*)(std::size_t index)`, which returns member names based on index.
            template <typename T> using member_name = impl::metadata_type_t<impl::value_member_names_func, T>;
        }

        namespace Custom
        {
            // Those are customization points for class reflection.
            // Here, `T` is never const.

            // Provides information about base classes.
            template <typename T, typename Void = void> struct bases
            {
                static_assert(std::is_void_v<Void>);
                using type = Meta::type_list<>;
            };
            template <typename T> struct bases<T, Meta::void_type<Macro::bases<T>>>
            {
                using type = Macro::bases<T>;
            };

            // Provides information about virtual base classes.
            template <typename T, typename Void = void> struct virt_bases
            {
                static_assert(std::is_void_v<Void>);
                using type = Meta::type_list<>;
            };
            template <typename T> struct virt_bases<T, Meta::void_type<Macro::virt_bases<T>>>
            {
                using type = Macro::virt_bases<T>;
            };

            // Provides information about members.
            template <typename T, typename Void = void> struct members
            {
                static_assert(std::is_void_v<Void>);
                static constexpr std::size_t count = -1;
            };
            template <typename T> struct members<T, Meta::void_type<Macro::member_ptrs<T>>>
            {
                static constexpr std::size_t count = Meta::list_size<Macro::member_ptrs<T>>;
                template <std::size_t I> static auto &at(T &object)
                {
                    return object.*Meta::list_value_at<Macro::member_ptrs<T>, I>;
                }
                template <std::size_t I> static const auto &at(const T &object)
                {
                    return object.*Meta::list_value_at<Macro::member_ptrs<T>, I>;
                }
            };

            namespace impl
            {
                // Converts a member index to the index of the corresponding `REFL_DECL`.
                template <std::size_t ...I, typename ...T>
                constexpr std::size_t MemberIndexToAttrPackIndex(Meta::type_list<Refl::impl::Class::Attr<I, T>...>, std::size_t index)
                {
                    std::size_t ret = 0;
                    (void)((index < I ? false : (index -= I, ret++, true)) && ...);
                    return ret;
                }
            }

            // Provides information about member attributes.
            template <typename T, typename Void = void> struct member_attribs
            {
                static_assert(std::is_void_v<Void>);
                template <std::size_t I> using at = Meta::type_list<>;
            };
            template <typename T> struct member_attribs<T, Meta::void_type<Macro::member_attribs<T>>>
            {
                template <std::size_t I>
                using at = typename Meta::list_type_at<Macro::member_attribs<T>, impl::MemberIndexToAttrPackIndex(Macro::member_attribs<T>{}, I)>::type;
            };

            // Provides information about member names.
            template <typename T, typename Void = void> struct member_names
            {
                static_assert(std::is_void_v<Void>);
                static constexpr bool known = false;
            };
            template <typename T> struct member_names<T, Meta::void_type<Macro::member_name<T>>>
            {
                static constexpr bool known = true;
                static const char *at(std::size_t index)
                {
                    // `index` is guaranteed to be valid.
                    return Macro::member_name<T>::value(index);
                }
            };
        }

        // This is the low-level interface.
        // Here, constness of T shouldn't matter.

        // Direct non-virtual and virtual bases.
        template <typename T> using bases = typename Custom::bases<std::remove_const_t<T>>::type;
        template <typename T> using virtual_bases = typename Custom::virt_bases<std::remove_const_t<T>>::type;

        // Non-staic members.
        template <typename T> inline constexpr bool members_known = Custom::members<std::remove_const_t<T>>::count != std::size_t(-1);
        template <typename T> inline constexpr std::size_t member_count = members_known<T> ? Custom::members<std::remove_const_t<T>>::count : 0;
        template <std::size_t I, Meta::deduce..., typename T> auto &Member(T &object)
        {
            return Custom::members<std::remove_const_t<T>>::template at<I>(object);
        }

        // Member attributes.
        // Returns a `Meta::type_list`.
        template <typename T, std::size_t I> using attrib_list = typename Custom::member_attribs<std::remove_const_t<T>>::template at<I>;

        // Member names.
        template <typename T> inline constexpr bool member_names_known = Custom::member_names<std::remove_const_t<T>>::known;
        template <typename T> [[nodiscard]] const char *MemberName(std::size_t i)
        {
            if (i >= member_count<T>)
                return "";
            return Custom::member_names<std::remove_const_t<T>>::at(i);
        }

        // This is the higher-level interface.

        // Returns the type of a member variable.
        template <typename T, std::size_t I> using member_type = std::remove_reference_t<decltype(Class::Member<I>(std::declval<T &>()))>; // Using a qualified call to protect against accidental ADL.

        namespace impl
        {
            template <typename Bases, typename Out> struct rec_virt_bases_norm {}; // Collect all recursive virtual bases of `Bases` to `Out`.
            template <typename VirtBases, typename Out> struct rec_virt_bases_virt {}; // Collect `VirtBases` and all their recursive virtual bases to `Out`.

            template <typename Out> struct rec_virt_bases_norm<Meta::type_list<>, Out> {using type = Out;};
            template <typename FirstBase, typename ...Bases, typename Out> struct rec_virt_bases_norm<Meta::type_list<FirstBase, Bases...>, Out>
            {
                using type =
                    typename rec_virt_bases_virt<virtual_bases<FirstBase>,  // ^ 3. Recursively process virtual bases of the first base.
                    typename rec_virt_bases_norm<bases<FirstBase>,          // | 2. Recursively process bases of the first base.
                    typename rec_virt_bases_norm<Meta::type_list<Bases...>, // | 1. Process the remaining bases.
                Out>::type>::type>::type;
            };

            template <typename Out> struct rec_virt_bases_virt<Meta::type_list<>, Out> {using type = Out;};
            template <typename FirstVirtBase, typename ...VirtBases, typename Out> struct rec_virt_bases_virt<Meta::type_list<FirstVirtBase, VirtBases...>, Out>
            {
                using type =
                    typename rec_virt_bases_virt<virtual_bases<FirstVirtBase>,  // ^ 4. Recursively process virtual bases of the first base.
                    typename rec_virt_bases_norm<bases<FirstVirtBase>,          // | 3. Recursively process bases of the first base.
                    typename rec_virt_bases_virt<Meta::type_list<VirtBases...>, // | 2. Process the remaining virtual bases.
                    Meta::list_copy_uniq<Meta::type_list<FirstVirtBase>,        // | 1. Add the first virtual base.
                Out>>::type>::type>::type;
            };
        }

        // Recursively get a list of all virtual bases of a class.
        template <typename T> using recursive_virtual_bases = typename impl::rec_virt_bases_virt<virtual_bases<T>, typename impl::rec_virt_bases_norm<bases<T>, Meta::type_list<>>::type>::type;
    }



    template <typename T>
    class Interface_Struct : public InterfaceBasic<T>
    {
      public:
        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options) const override
        {
            static_assert(Class::members_known<T>, "Can't convert T to string: its members are not reflected.");

            if constexpr (Class::member_names_known<T>)
            {
                output.WriteChar('{');

                auto next_options = options;
                if (options.multiline)
                    next_options.extra_indent += options.indent;

                bool first = true;

                auto WriteSeparator = [&]
                {
                    if (first)
                    {
                        first = false;
                        if (options.multiline)
                            output.WriteChar('\n').WriteChar(' ', next_options.extra_indent);
                    }
                    else
                    {
                        output.WriteChar(',');
                        if (options.multiline)
                            output.WriteChar('\n').WriteChar(' ', next_options.extra_indent);
                    }
                };

                auto WriteBase = [&](auto tag)
                {
                    using type = const typename decltype(tag)::type;
                    if (Class::member_count<type> == 0)
                        return;

                    type &ref = *static_cast<type *>(&object); // Use a pointer cast rather than a reference cast to avoid any custom conversion operators.
                    std::string name = Abi::TypeName<type>();

                    WriteSeparator();

                    Interface(name).ToString(name, output, {});
                    if (options.multiline)
                        output.WriteChar(' ');

                    Interface(ref).ToString(ref, output, next_options);
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
                    output.WriteString(Class::MemberName<T>(i));
                    if (options.multiline)
                        output.WriteString(" = ");
                    else
                        output.WriteChar('=');

                    Interface(ref).ToString(ref, output, next_options);
                });

                if (!first && options.multiline)
                    output.WriteString(",\n").WriteChar(' ', options.extra_indent);

                output.WriteChar('}');
            }
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

/* Macros for declaring reflected structures - the long guide.

Usage:


--- REFL_MEMBERS ---
If you want members of a struct to be reflected, you have to declare them using a macro:

    struct A
    {
        REFL_MEMBERS( <members> )
    };

Here, `<members>` is a sequence of zero or more entires, without delimiters.
Allowed entries are:
* REFL_DECL( <type> [<params>] ) <variables>
* REFL_VERBATIM <text>

Alternatively, you can use REFL_UNNAMED_MEMBERS instead of REFL_MEMBERS.
This macro has the same effect, except it doesn't save information about field names.

`REFL_MEMBERS` can be used at most once per structure.


--- REFL_DECL ---
Declares one or more member variables. Must be used inside of `REFL_MEMBERS`.
Usage:

    REFL_DECL( <type> [<params>] ) <variables>

Where:
<type> is the type of the variables,
<variables> is a comma-separated list of variable names,
<params> is zero or more optional parameters, without delimiters.

Allowed parameters are:

* `REFL_INIT <expression>`
  Sets an initializer for all variables declared by this `REFL_DECL`.
  The initializer must begin with `=`, or be enclosed in `{...}`.
  Can be used at most once per `REFL_DECL`.

* `REFL_ATTR <attributes>`
  Sets attributes for the variables.
  <attributes> is a comma-separated list of one or more attributes.
  Each attribute is a class type inherited from `Refl::BasicAttribute`.
  The attributes don't have any inherent meaning, but they can be queried using reflection.
  Can be used at most once per `REFL_DECL`.


--- REFL_VERBATIM ---
Puts a text in the class declaration verbatim. Must be used inside of `REFL_MEMBERS`.
Usage:

    REFL_VERBATIM <text>

For convenience, following macros are also provided:
REFL_PUBLIC    -> REFL_VERBATIM public
REFL_PRIVATE   -> REFL_VERBATIM private
REFL_PROTECTED -> REFL_VERBATIM protected


--- REFL_STRUCT ---
In addition to members, you can reflect other information about a struct,
such as base classes.
To do so, you have define it using `REFL_STRUCT`:

    REFL_STRUCT( <name> [<params>] )
    {
        // Struct body.
    };

Where:
<name> is the name of the structure.
<params> is zero or more optional parameters, without delimiters.

Allowed parameters are:

* `REFL_BASE <base_class>`
  Specifies a public base class for the structure.
  Can be used more than once per `REFL_STRUCT`.

* `REFL_VIRTUAL_BASE <base_class>`
  Same, but the base class is virtual.

* `REFL_UNTRACKED_BASES <base_classes>`
  Specifies additional base classes that shouldn't be visible to reflection.
  <base_classes> is a comma-separated list of one or more classes.
  Can be used at most once per `REFL_STRUCT`.

* `REFL_PARAM( <pseudo-type> ) <name> [, <init>]`
  Specifies a template parameter for the structure.
  If used at least once, the structure is a template.
  <pseudo-type> is either a type of a non-type parameter,
  or `[template< ... >] {typename|class}` for a type parameter.
  Can be used more than once per `REFL_STRUCT`.

* `REFL_POLYMORPHIC`
  Specifies that the structure is polymorphic, and has a virtual destructor.
  It's achieved by inheriting from `Meta::with_virtual_destructor<T>`.
  Can be used at most once per `REFL_STRUCT`.

* `REFL_FINAL`
  Specifies that the structure is `final`.
  Can be used at most once per `REFL_STRUCT`.

* `REFL_METADATA_ONLY`
  Prevents structure definition from being generated, but the metadata
  is still generated. See the next section.

* `REFL_TERSE <body>`
  Causes a terse structure definition syntax to be used.
  If it's used, `REFL_STRUCT` generates a simple structure
  body (with the trailing semicolon). If you use this parameter,
  you must not specify the body manually below the macro invocation.
  <body> is a list of members, interpreted as if passed to `REFL_MEMBERS`.
  It can be used at most once per `REFL_STRUCT`.
  This parameter is special. If present, it must be the last parameter.

* `REFL_TERSE_WITHOUT_NAMES`
  Same as `REFL_TERSE`, but doesn't generate metadata for the field names.


--- GENERATING METADATA ---
It's possible to reflect an existing structure.
To do so, you need to generate a metadata for it.

You can generate metadata by invoking `REFL_STRUCT` with
the `REFL_METADATA_ONLY` parameter.

Generating metadata is only possible using `REFL_TERSE[_WITHOUT_NAMES]`
(aka the terse struct definition syntax).

Note that some information is not used when generating reflection metadata.
For example, variable types are ignored, which allows you to
invoke `REFL_DECL` with no parameters.


--- METADATA FORMAT ---
Below is the description of the metadata generated by the macros.

Basic metadata, at the same scope as the class.

    struct zrefl_StructHelper_##StructName
    {
        // A list of bases.
        using bases = ::Meta::type_list<base1, base2, ...>;
        // A list of virtual bases.
        using virt_bases = ::Meta::type_list<base3, base4, ...>;

        // If both `REFL_METADATA_ONLY` and `REFL_TERSE` are used,
        // members metadata will also be here.
        // It's will look exactly like the contents of `struct Helper` described below.
    };
    inline static zrefl_StructHelper_##StructName zrefl_StructFunc(Meta::tag<StructName>) {return {};}

Members metadata, inside of a class.

    auto zrefl_MembersHelper() const
    {
        struct Helper
        {
            // Member pointers.
            using member_ptrs = Meta::value_list<&StructName::var1, &StructName::var2, ...>;

            // Member attributes.
            // Only present if there is at least one attribute used in the structure.
            // Here, `ATTRIBSi` is a set of attributes, and `COUNTi` is the amount of variables using those attributes.
            // One `Refl::impl::Class::Attr` corresponds to one `REFL_DECL`.
            using member_attribs = Meta::type_list<
                Refl::impl::Class::Attr<COUNT1, ATTRIBS1>,
                Refl::impl::Class::Attr<COUNT2, ATTRIBS2>
            >;

            // Member names.
            // Is present if not disabled when generating the metadata.
            static constexpr const char *member_name(std::size_t index)
            {
                constexpr const char *ret[] = {"var1", "var2", ..., nullptr};
                return ret[index];
            }
        };
        return Helper{};
    }

That's all.
*/


// An optional parameter for `REFL_STRUCT`.
// Specifies a base class for the struct.
#define REFL_BASE MA_PARAM(ReflBase)
#define MA_PARAMS_category_ReflStruct_X_ReflBase
#define MA_PARAMS_equal_ReflBase_X_ReflBase

// An optional parameter for `REFL_STRUCT`.
// Specifies a virtual base class for the struct.
#define REFL_VIRTUAL_BASE MA_PARAM(ReflVirtualBase)
#define MA_PARAMS_category_ReflStruct_X_ReflVirtualBase
#define MA_PARAMS_equal_ReflVirtualBase_X_ReflVirtualBase

// An optional parameter for `REFL_STRUCT`.
// Specifies a list of untracked bases for the struct.
// Can't be used more than once per struct declaration.
// 'Untracked' means that those bases are ignored by the reflection
#define REFL_UNTRACKED_BASES MA_PARAM(ReflUntrackedBases)
#define MA_PARAMS_category_ReflStruct_X_ReflUntrackedBases
#define MA_PARAMS_equal_ReflUntrackedBases_X_ReflUntrackedBases

// An optional parameter for `REFL_STRUCT`.
// Specifies a single template parameter for the struct.
// Must be followed by `(Type) Name [, Init]`.
#define REFL_PARAM(type) MA_PARAM(ReflTemplateParam) (type),
#define MA_PARAMS_category_ReflStruct_X_ReflTemplateParam
#define MA_PARAMS_equal_ReflTemplateParam_X_ReflTemplateParam

// An optional parameter for `REFL_STRUCT`.
// This struct is polymorphic.
#define REFL_POLYMORPHIC MA_PARAM(ReflIsPoly)
#define MA_PARAMS_category_ReflStruct_X_ReflIsPoly
#define MA_PARAMS_equal_ReflIsPoly_X_ReflIsPoly

// An optional parameter for `REFL_STRUCT`.
// This struct is `final`.
#define REFL_FINAL MA_PARAM(ReflIsFinal)
#define MA_PARAMS_category_ReflStruct_X_ReflIsFinal
#define MA_PARAMS_equal_ReflIsFinal_X_ReflIsFinal

// An optional parameter for `REFL_STRUCT`.
// If present, only the struct metadata will be generated, but not the struct itself.
#define REFL_METADATA_ONLY MA_PARAM(ReflMetadataOnly)
#define MA_PARAMS_category_ReflStruct_X_ReflMetadataOnly
#define MA_PARAMS_equal_ReflMetadataOnly_X_ReflMetadataOnly

// An optional parameter for `REFL_STRUCT`.
// If present, must be the last parameter.
// If present, replaces a struct body that would normally follow a `REFL_STRUCT` invocation.
// The parameters is interpreted as a struct body, as if it was passed to `REFL_MEMBERS`.
// `REFL_TERSE_WITHOUT_NAMES` is similar to `REFL_TERSE`, but uses `REFL_UNNAMED_MEMBERS`.
#define REFL_TERSE )),(,(
#define REFL_TERSE_WITHOUT_NAMES )),(x,(

// Declares a reflected struct, and some metadata for it.
// Must be followed by a struct body (unless `REFL_TERSE` parameter is present).
#define REFL_STRUCT(...) \
    REFL_STRUCT_impl(((__VA_ARGS__)),)

// Internal. Declares a reflected struct, and a metadata for it.
#define REFL_STRUCT_impl(seq, ...) \
    REFL_STRUCT_impl_low( \
        MA_PARAMS_FIRST(seq), \
        MA_PARAMS_GET(, ReflStruct, ReflTemplateParam, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET(, ReflStruct, ReflBase, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET(, ReflStruct, ReflVirtualBase, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflUntrackedBases, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflIsFinal, seq, MA_PARAMS_DUMMY_EMPTY), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflIsPoly, seq, MA_PARAMS_DUMMY_EMPTY), \
        __VA_OPT__(REFL_STRUCT_impl_get_body(__VA_ARGS__)), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflMetadataOnly, seq, MA_PARAMS_DUMMY_EMPTY) \
    )

// Internal. Helper for `REFL_STRUCT_impl`.
// Given `body,`, returns `body`. Emits an error if there is more than one comma.
#define REFL_STRUCT_impl_get_body(body, ...) \
    __VA_OPT__(MA_ABORT("Invalid usage of REFL_STRUCT(...).")) body

// Internal. Declares a reflected struct, and a metadata for it.
// Also generates a struct body if `REFL_TERSE` is present, but `REFL_METADATA_ONLY` is not.
// `name_` is a class name. `tparams_seq_` is a sequence of template parameters: `(type,name[,init])...` or empty,
// `[virt_]base_seq_` are sequences of base classes: `(name)...` or empty, `untracked_bases_` is `(base1,base2,...)` or empty,
// `*_if_not_empty_` are equal to `x` or empty, `body_or_empty_` is `(unnamed_if_not_empty_,seq_)` or empty.
#define REFL_STRUCT_impl_low(name_, tparams_seq_, base_seq_, virt_base_seq_, untracked_bases_, is_final_if_not_empty_, is_poly_if_not_empty_, body_or_empty_, metadata_only_if_not_empty_) \
    /* Unless we're generating metadata, declar the structure. */\
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_decl, metadata_only_if_not_empty_) \
        (name_, tparams_seq_) \
    /* Define the primary metadata structure. */\
    struct MA_CAT(zrefl_StructHelper_, name_) \
    { \
        /* A list of bases. */\
        using bases = ::Meta::type_list<MA_SEQ_TO_VA(base_seq_)>; \
        /* A list of virtual bases. */\
        using virt_bases = ::Meta::type_list<MA_SEQ_TO_VA(virt_base_seq_)>; \
        /* If both `REFL_METADATA_ONLY` and `REFL_TERSE` are used, generate metadata */\
        /* for member variables here instead of its normal location. */\
        MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_extra_metadata, MA_INVERT_EMPTINESS(body_or_empty_) MA_INVERT_EMPTINESS(metadata_only_if_not_empty_)) \
            (name_, MA_IDENTITY body_or_empty_) \
    }; \
    /* Define a helper function that returns the metadata structure. */\
    REFL_STRUCT_impl_tparams_decl(tparams_seq_) \
    [[maybe_unused]] inline static MA_CAT(zrefl_StructHelper_, name_) zrefl_StructFunc(::Meta::tag<name_ REFL_STRUCT_impl_tparams(tparams_seq_)>) {return {};} \
    /* Generate the beginning of the definition of the structure. */\
    /* It includes the struct name and a list of bases. */\
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_header, metadata_only_if_not_empty_) \
        (name_, tparams_seq_, base_seq_, virt_base_seq_, untracked_bases_, is_final_if_not_empty_, is_poly_if_not_empty_) \
    /* If `REFL_TERSE` is used (and `REFL_METADATA_ONLY` is not), generate a simple body for the structure. */\
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_body, MA_INVERT_EMPTINESS(body_or_empty_) metadata_only_if_not_empty_) \
        (MA_IDENTITY body_or_empty_)

// Internal. Helper for `REFL_STRUCT_impl_low`. Declares a structure, possibly with template parameters.
#define REFL_STRUCT_impl_low_decl(name_, tparams_seq_) \
    REFL_STRUCT_impl_tparams_firstdecl(tparams_seq_) struct name_;

// Internal. Helper for `REFL_STRUCT_impl_low`. Generates the beginning of the definition of a structure,
// which includes its name and base classes.
#define REFL_STRUCT_impl_low_header(name_, tparams_seq_, base_seq_, virt_base_seq_, untracked_bases_, is_final_if_not_empty_, is_poly_if_not_empty_) \
    REFL_STRUCT_impl_tparams_decl(tparams_seq_) struct name_ \
    MA_IF_NOT_EMPTY(final, is_final_if_not_empty_) \
    MA_IF_NOT_EMPTY_ELSE(REFL_STRUCT_impl_low_expand_bases, MA_NULL, base_seq_ virt_base_seq_ untracked_bases_ is_poly_if_not_empty_) \
    ( \
        name_ REFL_STRUCT_impl_tparams(tparams_seq_), \
        base_seq_, virt_base_seq_, untracked_bases_, \
        is_poly_if_not_empty_ \
    )

// Internal. Helper for `REFL_STRUCT_impl_low`. Generates a simple body for the structure (for `REFL_TERSE`).
#define REFL_STRUCT_impl_low_body(...) REFL_STRUCT_impl_low_body_low(__VA_ARGS__)
#define REFL_STRUCT_impl_low_body_low(unnamed_if_not_empty, seq) \
    { MA_IF_NOT_EMPTY_ELSE(REFL_UNNAMED_MEMBERS_impl, REFL_MEMBERS_impl, unnamed_if_not_empty)(seq) };

// Internal. Helper for `REFL_STRUCT_impl_low`. Used for generating field metadata if both `REFL_METADATA_ONLY` and `REFL_TERSE` are used.
#define REFL_STRUCT_impl_low_extra_metadata(...) REFL_STRUCT_impl_low_extra_metadata_low(__VA_ARGS__)
#define REFL_STRUCT_impl_low_extra_metadata_low(name, unnamed_if_not_empty, seq) \
    using t = name; \
    REFL_MEMBERS_impl_metadata_generic(seq) \
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_extra_metadata_low_names, unnamed_if_not_empty)(seq)

#define REFL_STRUCT_impl_low_extra_metadata_low_names(seq) REFL_MEMBERS_impl_metadata_memname(seq)


// Internal. Helper for `REFL_STRUCT_impl_low`. Generates a full list of template parameters (`template<...>`) that includes the default values.
// `seq` is `(type,name[,init])...`. If `seq` is empty, expands to nothing.
#define REFL_STRUCT_impl_tparams_firstdecl(seq) MA_IF_NOT_EMPTY(template<MA_APPEND_TO_VA_END(_end, REFL_STRUCT_impl_tparams_firstdecl_loop_0 seq)>, seq)
#define REFL_STRUCT_impl_tparams_firstdecl_loop_0(...)   REFL_STRUCT_impl_tparams_firstdecl_loop_body(__VA_ARGS__) REFL_STRUCT_impl_tparams_firstdecl_loop_a
#define REFL_STRUCT_impl_tparams_firstdecl_loop_a(...) , REFL_STRUCT_impl_tparams_firstdecl_loop_body(__VA_ARGS__) REFL_STRUCT_impl_tparams_firstdecl_loop_b
#define REFL_STRUCT_impl_tparams_firstdecl_loop_b(...) , REFL_STRUCT_impl_tparams_firstdecl_loop_body(__VA_ARGS__) REFL_STRUCT_impl_tparams_firstdecl_loop_a
#define REFL_STRUCT_impl_tparams_firstdecl_loop_0_end
#define REFL_STRUCT_impl_tparams_firstdecl_loop_a_end
#define REFL_STRUCT_impl_tparams_firstdecl_loop_b_end
#define REFL_STRUCT_impl_tparams_firstdecl_loop_body(type, ...) MA_IF_COMMA_ELSE(REFL_STRUCT_impl_tparams_firstdecl_loop_body3,REFL_STRUCT_impl_tparams_firstdecl_loop_body2,__VA_ARGS__)(type, __VA_ARGS__)
#define REFL_STRUCT_impl_tparams_firstdecl_loop_body3(type, name, /*init*/...) MA_IDENTITY type name = __VA_ARGS__
#define REFL_STRUCT_impl_tparams_firstdecl_loop_body2(type, name) MA_IDENTITY type name

// Internal. Helper for `REFL_STRUCT_impl_low`. Generates a list of template parameters (`template<...>`) without default values.
// `seq` is `(type,name[,init])...`. If `seq` is empty, expands to nothing.
#define REFL_STRUCT_impl_tparams_decl(seq) MA_IF_NOT_EMPTY(template<MA_APPEND_TO_VA_END(_end, REFL_STRUCT_impl_tparams_decl_loop_0 seq)>, seq)
#define REFL_STRUCT_impl_tparams_decl_loop_0(...)   REFL_STRUCT_impl_tparams_decl_loop_body(__VA_ARGS__,) REFL_STRUCT_impl_tparams_decl_loop_a
#define REFL_STRUCT_impl_tparams_decl_loop_a(...) , REFL_STRUCT_impl_tparams_decl_loop_body(__VA_ARGS__,) REFL_STRUCT_impl_tparams_decl_loop_b
#define REFL_STRUCT_impl_tparams_decl_loop_b(...) , REFL_STRUCT_impl_tparams_decl_loop_body(__VA_ARGS__,) REFL_STRUCT_impl_tparams_decl_loop_a
#define REFL_STRUCT_impl_tparams_decl_loop_0_end
#define REFL_STRUCT_impl_tparams_decl_loop_a_end
#define REFL_STRUCT_impl_tparams_decl_loop_b_end
#define REFL_STRUCT_impl_tparams_decl_loop_body(type, name, ...) MA_IDENTITY type name

// Internal. Helper for `REFL_STRUCT_impl_low`. Generates a short list of template parameters (`<...>`) that includes only the parameter names.
// `seq` is `(type,name[,init])...`. If `seq` is empty, expands to nothing.
#define REFL_STRUCT_impl_tparams(seq) MA_IF_NOT_EMPTY(<MA_APPEND_TO_VA_END(_end, REFL_STRUCT_impl_tparams_loop_0 seq)>, seq)
#define REFL_STRUCT_impl_tparams_loop_0(...)   REFL_STRUCT_impl_tparams_loop_body(__VA_ARGS__,) REFL_STRUCT_impl_tparams_loop_a
#define REFL_STRUCT_impl_tparams_loop_a(...) , REFL_STRUCT_impl_tparams_loop_body(__VA_ARGS__,) REFL_STRUCT_impl_tparams_loop_b
#define REFL_STRUCT_impl_tparams_loop_b(...) , REFL_STRUCT_impl_tparams_loop_body(__VA_ARGS__,) REFL_STRUCT_impl_tparams_loop_a
#define REFL_STRUCT_impl_tparams_loop_0_end
#define REFL_STRUCT_impl_tparams_loop_a_end
#define REFL_STRUCT_impl_tparams_loop_b_end
#define REFL_STRUCT_impl_tparams_loop_body(type, name, ...) name


// Internal. Helper for `REFL_STRUCT_impl_low`. Generates a list of base classes, starting with a colon.
// `name` is the struct name, possibly followed by `<...>` template parameters.
// `[virt_]base_seq` are sequences of base classes: `(class1)(class2)...` or empty,
// `untracked_bases` is a list of base classes: `(class1,class2,...)` or empty.
// `...` is `x` if the class is polymorphic, empty otherwise.
#define REFL_STRUCT_impl_low_expand_bases(name, base_seq, virt_base_seq, untracked_bases_, /*is_poly_if_not_empty*/...) \
    : MA_IDENTITY2( /* can't use `MA_IDENTITY` here, since it would conflict with the same macro in `REFL_STRUCT_impl_low_base` */\
        MA_NULL \
        /* Regular bases, including the special base that makes the struct polymorphic if needed. */\
        MA_SEQ_FOR_EACH(, REFL_STRUCT_impl_low_base, __VA_OPT__((::Meta::with_virtual_destructor<name>)) base_seq) \
        /* Virtual bases. */\
        MA_SEQ_FOR_EACH(virtual, REFL_STRUCT_impl_low_base, virt_base_seq) \
        /* Untracked bases. */\
        MA_IF_NOT_EMPTY(REFL_STRUCT_impl_low_base(,,MA_IDENTITY untracked_bases_), untracked_bases_) \
        () \
    )

// Internal. Helper for `REFL_STRUCT_impl_low_expand_bases`. Expands to a single base class of a struct.
#define REFL_STRUCT_impl_low_base(maybe_virtual, index, ...) (,) maybe_virtual __VA_ARGS__ MA_IDENTITY


// An optional parameter for `REFL_DECL`.
// Specifies an initializer for each member created by this declaration.
// The initializer either has to begin with `=` or be enclosed in `{...}`.
#define REFL_INIT MA_PARAM(ReflInit)
#define MA_PARAMS_category_ReflMemberDecl_X_ReflInit
#define MA_PARAMS_equal_ReflInit_X_ReflInit

// An optional parameter for `REFL_DECL`.
// Specifies a list of arbitrary classes (derived from `Refl::BasicAttribute`) as attributes of a set of variables.
// Attributes don't have any inherent meaning, but can be queried.
#define REFL_ATTR MA_PARAM(ReflAttr)
#define MA_PARAMS_category_ReflMemberDecl_X_ReflAttr
#define MA_PARAMS_equal_ReflAttr_X_ReflAttr

// Can be passed to `REFL_MEMBERS`.
// Declares one or more variables.
// Usage: `REFL_DECL( Type [Parameters] ) var1,var2,var3`
#define REFL_DECL(...) )(((__VA_ARGS__)),

// Can be passed to `REFL_MEMBERS`.
// Indicates that the following text should be added to the class declaration verbatim.
#define REFL_VERBATIM )(,

// Some shorthands for `REFL_VERBATIM`.
#define REFL_PUBLIC REFL_VERBATIM public
#define REFL_PRIVATE REFL_VERBATIM private
#define REFL_PROTECTED REFL_VERBATIM protected

// Declares several member variables of a struct, as well as some metadata for them.
// Must be used at class scope, at most once per class.
// `...` is a sequence of `REFL_DECL` and `REFL_VERBATIM` entries.
#define REFL_MEMBERS(...) REFL_MEMBERS_impl((__VA_ARGS__))
#define REFL_MEMBERS_impl(...) \
    REFL_MEMBERS_impl_decl(__VA_ARGS__) \
    auto zrefl_MembersHelper() const \
    { \
        using t [[maybe_unused]] = std::remove_cv_t<std::remove_pointer_t<decltype(this)>>; \
        struct Helper \
        { \
            REFL_MEMBERS_impl_metadata_generic(__VA_ARGS__) \
            REFL_MEMBERS_impl_metadata_memname(__VA_ARGS__) \
        }; \
        return Helper{}; \
    }

// Same as `REFL_MEMBERS`, but doesn't save variable names.
#define REFL_UNNAMED_MEMBERS(...) REFL_UNNAMED_MEMBERS_impl((__VA_ARGS__))
#define REFL_UNNAMED_MEMBERS_impl(...) \
    REFL_MEMBERS_impl_decl(__VA_ARGS__) \
    auto zrefl_MembersHelper() const \
    { \
        using t = std::remove_cv_t<std::remove_pointer_t<decltype(this)>>; \
        struct Helper \
        { \
            REFL_MEMBERS_impl_metadata_generic(__VA_ARGS__) \
        }; \
        return Helper{}; \
    }


// Internal. Helper for `REFL_MEMBERS`. Declares variables themselves, without metadata.
#define REFL_MEMBERS_impl_decl(...) REFL_MEMBERS_impl_decl_low(REFL_MEMBERS_impl_skip_first __VA_ARGS__)
#define REFL_MEMBERS_impl_skip_first(...) __VA_OPT__(MA_ABORT("Junk before the first entry in a `REFL_MEMBERS` invocation."))

#define REFL_MEMBERS_impl_decl_low(...) MA_APPEND_TO_VA_END(_end, REFL_MEMBERS_impl_decl_loop_a __VA_ARGS__ )

#define REFL_MEMBERS_impl_decl_loop_a(...) REFL_MEMBERS_impl_decl_body(__VA_ARGS__) REFL_MEMBERS_impl_decl_loop_b
#define REFL_MEMBERS_impl_decl_loop_b(...) REFL_MEMBERS_impl_decl_body(__VA_ARGS__) REFL_MEMBERS_impl_decl_loop_a
#define REFL_MEMBERS_impl_decl_loop_a_end
#define REFL_MEMBERS_impl_decl_loop_b_end

// Internal. Helper for `REFL_MEMBERS_impl_decl`. Called for each `REFL_DECL` and `REFL_VERBATIM`.
// `params` is a list of named parameters of `REFL_DECL`, or nothing for `REFL_VERBATIM`.
// For `REFL_DECL`, generates member variables without metadata.
// For `REFL_VERBATIM`, inserts the text verbatim.
#define REFL_MEMBERS_impl_decl_body(params, ...) \
    MA_IF_NOT_EMPTY_ELSE(REFL_MEMBERS_impl_decl_body_low_members, REFL_MEMBERS_impl_decl_body_low_text, params)(params, __VA_ARGS__)

// Internal. Helper for `REFL_MEMBERS_impl_decl_body`. Inserts the contents of a `REFL_VERBATIM` verbatim.
#define REFL_MEMBERS_impl_decl_body_low_text(params, ...) __VA_ARGS__

// Internal. Helper for `REFL_MEMBERS_impl_decl_body`. Declares the variables described by a `REFL_DECL`.
#define REFL_MEMBERS_impl_decl_body_low_members(params, ...) \
    ::Meta::identity_t<MA_PARAMS_FIRST(params)> /* type */\
    MA_NULL MA_VA_FOR_EACH( /* variables */\
        (MA_PARAMS_GET_ONE(, ReflMemberDecl, ReflInit, params, MA_PARAMS_IDENTITY)), /* initializer */\
        REFL_MEMBERS_impl_decl_member, \
        MA_TR_C(__VA_ARGS__) /* variable names */\
    ) () \
    ;

// Internal. Helper for `REFL_MEMBERS_impl_decl_body_low_members`. Expands to a single variable name, possibly with an initializer.
#define REFL_MEMBERS_impl_decl_member(init, index, name) (,) name MA_IDENTITY init MA_IDENTITY

// Internal. Generates common metadata for member variables.
// `...` is a list of `REFL_DECL` and `REFL_VERBATIM` entries.
// This metadata includes member pointers, and attributes if they are present.
#define REFL_MEMBERS_impl_metadata_generic(...) \
    MA_CALL(REFL_MEMBERS_impl_metadata_generic_low, REFL_MEMBERS_impl_skip_first __VA_ARGS__)

#define REFL_MEMBERS_impl_metadata_generic_low(...) \
    using member_ptrs = ::Meta::value_list< \
        /* can't use `MA_IDENTITY` here, since it would conflict with the same macro in `REFL_STRUCT_impl_low_base` */\
        MA_IDENTITY2( MA_NULL MA_APPEND_TO_VA_END(_end, REFL_MEMBERS_impl_metadata_memptr_loop_a __VA_ARGS__) () ) \
    >; \
    MA_IF_NOT_EMPTY_ELSE(REFL_MEMBERS_impl_metadata_generic_low_attribs, MA_NULL, \
    MA_APPEND_TO_VA_END(_end, REFL_MEMBERS_impl_metadata_memattr_dry_loop_a __VA_ARGS__))(__VA_ARGS__)

// Internal. Helper for `REFL_MEMBERS_impl_metadata_generic_low`.
// Generates an attribute list as a part of the common metadata for member variables.
#define REFL_MEMBERS_impl_metadata_generic_low_attribs(...) \
    using member_attribs = ::Meta::type_list< \
        MA_APPEND_TO_VA_END(_end, REFL_MEMBERS_impl_metadata_memattr_loop_0 __VA_ARGS__) \
    >;


// Internal. Helper for `REFL_MEMBERS_impl_metadata_generic_low`. Generates member pointers (as a part of common metadata).
#define REFL_MEMBERS_impl_metadata_memptr_loop_a(...) REFL_MEMBERS_impl_metadata_memptr_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memptr_loop_b
#define REFL_MEMBERS_impl_metadata_memptr_loop_b(...) REFL_MEMBERS_impl_metadata_memptr_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memptr_loop_a
#define REFL_MEMBERS_impl_metadata_memptr_loop_a_end
#define REFL_MEMBERS_impl_metadata_memptr_loop_b_end

#define REFL_MEMBERS_impl_metadata_memptr_body(params, ...) \
    MA_IF_NOT_EMPTY_ELSE(REFL_MEMBERS_impl_metadata_memptr_body_low, MA_NULL, params)(params, __VA_ARGS__)

#define REFL_MEMBERS_impl_metadata_memptr_body_low(params, ...) MA_VA_FOR_EACH(, REFL_MEMBERS_impl_metadata_memptr_pointer, MA_TR_C(__VA_ARGS__) )
#define REFL_MEMBERS_impl_metadata_memptr_pointer(data, index, name) (,) &t::name MA_IDENTITY

// Internal. Helper for `REFL_MEMBERS_impl_metadata_generic_low`.
// Checks if at least one attribute used in a struct. If there is, expands to one or more `x` symbols, otherwise expands to nothing.
#define REFL_MEMBERS_impl_metadata_memattr_dry_loop_a(...) REFL_MEMBERS_impl_metadata_memattr_dry_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memattr_dry_loop_b
#define REFL_MEMBERS_impl_metadata_memattr_dry_loop_b(...) REFL_MEMBERS_impl_metadata_memattr_dry_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memattr_dry_loop_a
#define REFL_MEMBERS_impl_metadata_memattr_dry_loop_a_end
#define REFL_MEMBERS_impl_metadata_memattr_dry_loop_b_end
#define REFL_MEMBERS_impl_metadata_memattr_dry_body(params, ...) MA_PARAMS_GET_ONE(, ReflMemberDecl, ReflAttr, params, MA_PARAMS_DUMMY)

// Internal. Helper for `REFL_MEMBERS_impl_metadata_generic_low`.
// Generates attribute information for member variables.
#define REFL_MEMBERS_impl_metadata_memattr_loop_0(...)   REFL_MEMBERS_impl_metadata_memattr_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memattr_loop_a
#define REFL_MEMBERS_impl_metadata_memattr_loop_a(...) , REFL_MEMBERS_impl_metadata_memattr_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memattr_loop_b
#define REFL_MEMBERS_impl_metadata_memattr_loop_b(...) , REFL_MEMBERS_impl_metadata_memattr_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memattr_loop_a
#define REFL_MEMBERS_impl_metadata_memattr_loop_0_end
#define REFL_MEMBERS_impl_metadata_memattr_loop_a_end
#define REFL_MEMBERS_impl_metadata_memattr_loop_b_end

// Internal. Helper for `REFL_MEMBERS_impl_metadata_memattr_loop_*`. Called once for each `DECL_REFL` for structs that use attributes.
// Expands to `::Refl::impl::Class::Attr<count, attribs>`, where `count` is the amount of variables in this `DECL_REFL` (represented as repeated `+1`),
// and `attribs` is a list of attributes.
#define REFL_MEMBERS_impl_metadata_memattr_body(params, ...) \
    MA_IF_NOT_EMPTY_ELSE(REFL_MEMBERS_impl_metadata_memattr_body_low, MA_NULL, params)(MA_PARAMS_GET_ONE(, ReflMemberDecl, ReflAttr, params, MA_PARAMS_PARENS), __VA_ARGS__)

#define REFL_MEMBERS_impl_metadata_memattr_body_low(maybe_attr, ...) \
    ::Refl::impl::Class::Attr< \
        MA_VA_FOR_EACH(, REFL_MEMBERS_impl_metadata_memattr_plus1, MA_TR_C(__VA_ARGS__)), \
        ::Meta::type_list<MA_IF_NOT_EMPTY(MA_IDENTITY maybe_attr, maybe_attr)> \
    >

#define REFL_MEMBERS_impl_metadata_memattr_plus1(data, index, name) +1


// Internal. Generates metadata for member names. (Not included in the common metadata).
#define REFL_MEMBERS_impl_metadata_memname(...) \
    MA_CALL(REFL_MEMBERS_impl_metadata_memname_low, REFL_MEMBERS_impl_skip_first __VA_ARGS__)

#define REFL_MEMBERS_impl_metadata_memname_low(...) \
    /* Can't use a static array here, because static arrays are not allowed in classes declared inside of functions. */\
    static constexpr const char *member_name(::std::size_t index) \
    { \
        constexpr const char *ret[] = { \
            MA_APPEND_TO_VA_END(_end, REFL_MEMBERS_impl_metadata_memname_loop_a __VA_ARGS__) \
            nullptr /* an extra terminating element prevents us from getting an empty array if there are no members */\
        }; \
        return ret[index]; \
    }

#define REFL_MEMBERS_impl_metadata_memname_loop_a(...) REFL_MEMBERS_impl_metadata_memname_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memname_loop_b
#define REFL_MEMBERS_impl_metadata_memname_loop_b(...) REFL_MEMBERS_impl_metadata_memname_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memname_loop_a
#define REFL_MEMBERS_impl_metadata_memname_loop_a_end
#define REFL_MEMBERS_impl_metadata_memname_loop_b_end

#define REFL_MEMBERS_impl_metadata_memname_body(params, ...) \
    MA_IF_NOT_EMPTY_ELSE(REFL_MEMBERS_impl_metadata_memname_body_low, MA_NULL, params)(params, __VA_ARGS__)

#define REFL_MEMBERS_impl_metadata_memname_body_low(params, ...) MA_VA_FOR_EACH(, REFL_MEMBERS_impl_metadata_memname_string, MA_TR_C(__VA_ARGS__) )

#define REFL_MEMBERS_impl_metadata_memname_string(data, index, name) #name,


#if 0 // Tests

REFL_STRUCT(MyStruct REFL_UNTRACKED_BASES std::vector<int> REFL_BASE std::string REFL_BASE std::string_view REFL_VIRTUAL_BASE std::vector<float>)
{

};

REFL_STRUCT(X REFL_PARAM(template <typename> typename) T,std::vector REFL_PARAM(auto) V,0 REFL_POLYMORPHIC )
{

};

REFL_STRUCT(A)
{
    REFL_MEMBERS(
        REFL_DECL(int REFL_INIT =0) x, y, z
        REFL_DECL(float) w, ww
        REFL_DECL(float) h, hh
    )
};

struct MyAttr : Refl::BasicAttribute {};

REFL_STRUCT(B)
{
    REFL_MEMBERS(
        REFL_DECL(int REFL_INIT =0) x, y, z
        REFL_DECL(float) w, ww
        REFL_DECL(float REFL_ATTR MyAttr) h, hh
    )
};

REFL_STRUCT( C
    REFL_TERSE
    REFL_DECL(int) x,y
    REFL_DECL(float) z
)

REFL_STRUCT( D
    REFL_TERSE_WITHOUT_NAMES
    REFL_DECL(int) x,y
    REFL_DECL(float) z
)

struct E {int x, y; float z;};

REFL_STRUCT( E
    REFL_METADATA_ONLY
    REFL_TERSE_WITHOUT_NAMES
    REFL_DECL() x,y
    REFL_DECL() z
)

#endif
