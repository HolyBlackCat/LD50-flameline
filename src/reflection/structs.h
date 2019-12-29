#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "reflection/interface_std_string.h"
#include "utils/macro.h"
#include "utils/meta.h"
#include "utils/named_macro_parameters.h"
#include "utils/strings.h"

namespace Refl
{
    struct BasicAttribute {}; // A base class for field attributes.
    struct BasicClassAttribute {}; // A base class for class attributes.

    inline namespace Attribs
    {
        // Affects deserializing objects from strings.
        // When used as a field attribute, makes the field optional.
        // When used as a class attribute of a base class, makes the base optional when deserializing a derived class.
        struct Optional : BasicAttribute, BasicClassAttribute {};
    }

    namespace impl::Class
    {
        // Stores the list of attributes for a single `REFL_DECL` declaration.
        // `N` is the amount of variables, `T` is a `Meta::type_list` of attributes.
        template <std::size_t N, typename T> struct Attr {};
        template <std::size_t N, typename ...P> struct Attr<N, Meta::type_list<P...>>
        {
            static_assert(((std::is_base_of_v<BasicAttribute, P> && !std::is_same_v<BasicAttribute, P>) && ...), "Attributes must inherit from Refl::BasicAttribute.");
            using list = Meta::type_list<P...>;
        };

        // Stores the list of attributes of a class.
        template <typename ...P> struct ClassAttr
        {
            static_assert(((std::is_base_of_v<BasicClassAttribute, P> && !std::is_same_v<BasicClassAttribute, P>) && ...), "Class attributes must inherit from Refl::BasicClassAttribute.");
            using list = Meta::type_list<P...>;
        };
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
                template <typename T, typename = void> struct member_metadata {}; // WARNING: If you rename this class, don't forget to rename the friend declaration in the macros.
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
                template <typename T> using value_name = Meta::value_tag<T::name>;
                template <typename T> using type_class_attribs = typename T::attribs;
                template <typename T> using type_bases = typename T::bases;
                template <typename T> using type_virt_bases = typename T::virt_bases;
                template <typename T> using type_member_ptrs = typename T::member_ptrs;
                template <typename T> using type_member_attribs = typename T::member_attribs;
                template <typename T> using value_member_names_func = Meta::value_tag<T::member_name>;
            }

            // All of those should be SFINAE-friendly.
            // `::value` is a const char pointer to the class name.
            template <typename T> using name = impl::metadata_type_t<impl::value_name, T>;
            // Returns attributes of a class as a `Refl::impl::Class::ClassAttr`.
            template <typename T> using class_attribs = impl::metadata_type_t<impl::type_class_attribs, T>;
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

            // Provides information about the class name.
            template <typename T, typename Void = void> struct name
            {
                static_assert(std::is_void_v<Void>);
                static constexpr const char *value = nullptr;
            };
            template <typename T> struct name<T, Meta::void_type<Macro::name<T>>>
            {
                static constexpr const char *value = Macro::name<T>::value;
            };

            // Provides information about class attributes.
            template <typename T, typename Void = void> struct class_attribs
            {
                static_assert(std::is_void_v<Void>);
                using type = Meta::type_list<>;
            };
            template <typename T> struct class_attribs<T, Meta::void_type<Macro::class_attribs<T>>>
            {
                using type = typename Macro::class_attribs<T>::list;
            };

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
                using at = typename Meta::list_type_at<typename Macro::member_attribs<T>::list, impl::MemberIndexToAttrPackIndex(Macro::member_attribs<T>{}, I)>::type;
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

        // Class name. (Unqualified, sans template parameters.)
        template <typename T> inline constexpr bool name_known = bool(Custom::name<std::remove_const_t<T>>::value);
        template <typename T> inline constexpr const char *name = Custom::name<std::remove_const_t<T>>::value;

        // Class attributes.
        // Returns a `Meta::type_list`.
        template <typename T> using class_attribs = typename Custom::class_attribs<std::remove_const_t<T>>::type;

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
        template <typename T, std::size_t I> using member_attribs = typename Custom::member_attribs<std::remove_const_t<T>>::template at<I>;

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

        // Check if a class has a specific attribute.
        template <typename T, typename A> inline constexpr bool class_has_attrib = Meta::list_contains_type<class_attribs<T>, A>;
        // Check if a class member has a specific attribute.
        template <typename T, std::size_t I, typename A> inline constexpr bool member_has_attrib = Meta::list_contains_type<member_attribs<T, I>, A>;
    }
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

* `REFL_EXTENDS <base_classes>`
  Specifies public base classes for the structure.
  <base_classes> is a comma-separated list of one or more classes.
  Can be used at most once per `REFL_STRUCT`.

* `REFL_VIRTUALLY_EXTENDS <base_classes>`
  Same as `REFL_EXTENDS`, but the base classes are virtual.

* `REFL_SILENTLY_EXTENDS <base_class_list>`
  Specifies additional base classes that shouldn't be visible to reflection.
  <base_class_list> is a comma-separated list of one or more classes that
  can include access specifiers and `virtual`.
  Can be used at most once per `REFL_STRUCT`.

* `REFL_ASSUME_EXTENDS <base_classes>`
  Same as `REFL_EXTENDS`, but the base classes are only added to the metadata.
  They are not actually inherited from. This is good for when you inherit from
  those classes indirectly, in a way that's otherwise not recorded in the metadata.

* `REFL_ASSUME_VIRTUALLY_EXTENDS <base_classes>`
  Same as `REFL_ASSUME_EXTENDS`, but the base classes are virtual.

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
        // Name.
        static constexpr char name[] = "StructName";
        // Class attributes.
        using attribs = Refl::impl::Class::ClassAttr<ATTRIBS>;
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
// Specifies a list of base classes for the struct.
// Can't be used more than once per struct declaration.
#define REFL_EXTENDS MA_PARAM(ReflBases)
#define MA_PARAMS_category_ReflStruct_X_ReflBases
#define MA_PARAMS_equal_ReflBases_X_ReflBases

// An optional parameter for `REFL_STRUCT`.
// Specifies a list of virtual base classes for the struct.
// Can't be used more than once per struct declaration.
#define REFL_VIRTUALLY_EXTENDS MA_PARAM(ReflVirtualBases)
#define MA_PARAMS_category_ReflStruct_X_ReflVirtualBases
#define MA_PARAMS_equal_ReflVirtualBases_X_ReflVirtualBases

// An optional parameter for `REFL_STRUCT`.
// Specifies a list of untracked base classes for the struct.
// Those bases will be invisible for the reflection (will not be added to metadata).
// Can't be used more than once per struct declaration.
#define REFL_SILENTLY_EXTENDS MA_PARAM(ReflUntrackedBases)
#define MA_PARAMS_category_ReflStruct_X_ReflUntrackedBases
#define MA_PARAMS_equal_ReflUntrackedBases_X_ReflUntrackedBases

// An optional parameter for `REFL_STRUCT`.
// Specifies a list of base classes for the struct that should be added
// to the metadata, but not actually inherited from. Good for when you
// indirectly inherit from those classes in a way that can't normally be detected.
// Can't be used more than once per struct declaration.
#define REFL_ASSUME_EXTENDS MA_PARAM(ReflFakeBases)
#define MA_PARAMS_category_ReflStruct_X_ReflFakeBases
#define MA_PARAMS_equal_ReflFakeBases_X_ReflFakeBases

// An optional parameter for `REFL_STRUCT`.
// Specifies a list of virtual base classes for the struct that should be added
// to the metadata, but not actually inherited from. Good for when you
// indirectly inherit from those classes in a way that can't normally be detected.
// Can't be used more than once per struct declaration.
#define REFL_ASSUME_VIRTUALLY_EXTENDS MA_PARAM(ReflFakeVirtualBases)
#define MA_PARAMS_category_ReflStruct_X_ReflFakeVirtualBases
#define MA_PARAMS_equal_ReflFakeVirtualBases_X_ReflFakeVirtualBases

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
// If this struct is used as a base, you'll be able to omit it when deserializing the derived struct from a string.
#define REFL_ATTR MA_PARAM(ReflAttr)
#define MA_PARAMS_category_ReflStruct_X_ReflAttr
#define MA_PARAMS_equal_ReflAttr_X_ReflAttr

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
        MA_PARAMS_GET_ONE(, ReflStruct, ReflBases, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflVirtualBases, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflFakeBases, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflFakeVirtualBases, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflUntrackedBases, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflIsPoly, seq, MA_PARAMS_DUMMY_EMPTY), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflIsFinal, seq, MA_PARAMS_DUMMY_EMPTY), \
        (MA_PARAMS_GET_ONE(, ReflStruct, ReflAttr, seq, MA_PARAMS_IDENTITY)), \
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
// `*bases_` are lists of base classes: `(base1,base2,...)` or empty, `*_if_not_empty_` are equal to `x` or empty,
// `body_or_empty_` is `(unnamed_if_not_empty_,seq_)` or empty.
#define REFL_STRUCT_impl_low(name_, tparams_seq_, bases_, virt_bases_, fake_bases_, fake_virt_bases_, untracked_bases_, is_poly_if_not_empty_, is_final_if_not_empty_, attribs_, body_or_empty_, metadata_only_if_not_empty_) \
    /* Unless we're generating metadata, declar the structure. */\
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_decl, metadata_only_if_not_empty_) \
        (name_, tparams_seq_) \
    /* Define the primary metadata structure. */\
    struct MA_CAT(zrefl_StructHelper_, name_) \
    { \
        /* A name. */\
        /* (Note that an array is used instead of a pointer. A pointer couldn't be put into */\
        /* a `Meta::value_tag`, so we'd need to change the implementation of `Refl::Class::Macro::name`.) */\
        static constexpr char name[] = MA_STR(name_); \
        /* Attribute list. */\
        using attribs = ::Refl::impl::Class::ClassAttr<MA_IDENTITY attribs_>; \
        /* A list of bases. */\
        using bases = ::Meta::type_list<MA_IF_NOT_EMPTY(MA_IDENTITY bases_, bases_) MA_IF_NOT_EMPTY(MA_IF_NOT_EMPTY(MA_COMMA(), bases_) MA_IDENTITY fake_bases_, fake_bases_)>; \
        /* A list of virtual bases. */\
        using virt_bases = ::Meta::type_list<MA_IF_NOT_EMPTY(MA_IDENTITY virt_bases_, virt_bases_) MA_IF_NOT_EMPTY(MA_IF_NOT_EMPTY(MA_COMMA(), virt_bases_) MA_IDENTITY fake_virt_bases_, fake_virt_bases_)>; \
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
        (name_, tparams_seq_, bases_, virt_bases_, untracked_bases_, is_poly_if_not_empty_, is_final_if_not_empty_) \
    /* If `REFL_TERSE` is used (and `REFL_METADATA_ONLY` is not), generate a simple body for the structure. */\
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_body, MA_INVERT_EMPTINESS(body_or_empty_) metadata_only_if_not_empty_) \
        (MA_IDENTITY body_or_empty_)

// Internal. Helper for `REFL_STRUCT_impl_low`. Declares a structure, possibly with template parameters.
#define REFL_STRUCT_impl_low_decl(name_, tparams_seq_) \
    REFL_STRUCT_impl_tparams_firstdecl(tparams_seq_) struct name_;

// Internal. Helper for `REFL_STRUCT_impl_low`. Generates the beginning of the definition of a structure,
// which includes its name and base classes.
#define REFL_STRUCT_impl_low_header(name_, tparams_seq_, bases_, virt_bases_, untracked_bases_, is_poly_if_not_empty_, is_final_if_not_empty_) \
    REFL_STRUCT_impl_tparams_decl(tparams_seq_) struct name_ \
    MA_IF_NOT_EMPTY(final, is_final_if_not_empty_) \
    MA_IF_NOT_EMPTY_ELSE(REFL_STRUCT_impl_low_expand_bases, MA_NULL, bases_ virt_bases_ untracked_bases_ is_poly_if_not_empty_) \
    ( \
        name_ REFL_STRUCT_impl_tparams(tparams_seq_), \
        bases_, virt_bases_, untracked_bases_, \
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
// `*bases` are lists of base classes: `(class1,class2,...)` or empty.
// `...` is `x` if the class is polymorphic, empty otherwise.
#define REFL_STRUCT_impl_low_expand_bases(name, bases, virt_bases, untracked_bases, /*is_poly_if_not_empty*/...) \
    : MA_IDENTITY2( /* can't use `MA_IDENTITY` here, since it would conflict with the same macro in `REFL_STRUCT_impl_low_base` */\
        MA_NULL \
        /* Regular bases. */\
        MA_IF_NOT_EMPTY(REFL_STRUCT_impl_low_base(,,MA_IDENTITY bases), bases) \
        /* Virtual bases. */\
        MA_IF_NOT_EMPTY(MA_VA_FOR_EACH(virtual, REFL_STRUCT_impl_low_base, MA_TR_C(MA_IDENTITY virt_bases)), virt_bases) \
        /* Untracked bases. */\
        MA_IF_NOT_EMPTY(REFL_STRUCT_impl_low_base(,,MA_IDENTITY untracked_bases), untracked_bases) \
        /* The special base that makes the struct polymorphic if needed. */\
        __VA_OPT__(REFL_STRUCT_impl_low_base(,,::Meta::with_virtual_destructor<name>)) \
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
//   #define REFL_ATTR MA_PARAM(ReflAttr)        // Already defined above.
#define MA_PARAMS_category_ReflMemberDecl_X_ReflAttr
//   #define MA_PARAMS_equal_ReflAttr_X_ReflAttr // Already defined above.

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
    } \
    template <typename, typename> friend struct ::Refl::Class::Macro::impl::member_metadata;

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

#define REFL_MEMBERS_impl_metadata_memattr_dry_body(params, ...) MA_IF_NOT_EMPTY_ELSE(REFL_MEMBERS_impl_metadata_memattr_dry_body_low, MA_NULL, params)(params)
#define REFL_MEMBERS_impl_metadata_memattr_dry_body_low(params) MA_PARAMS_GET_ONE(, ReflMemberDecl, ReflAttr, params, MA_PARAMS_DUMMY)

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

REFL_STRUCT(MyStruct REFL_SILENTLY_EXTENDS std::vector<int> REFL_EXTENDS std::string, std::string_view REFL_VIRTUALLY_EXTENDS std::vector<float>)
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
