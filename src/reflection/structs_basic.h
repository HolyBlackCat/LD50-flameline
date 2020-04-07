#pragma once

#include <algorithm>
#include <array>
#include <cstring>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "macros/generated.h"
#include "macros/named_macro_parameters.h"
#include "meta/misc.h"
#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "reflection/interface_std_string.h"
#include "strings/common.h"

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
                // Combines stuff related to the internal (member) metadata, one of the two possible metadata types.
                // We use a class to be able to `friend` it all at once.
                struct member_metadata_helpers
                {
                    // Given a member function (const-qualified) pointer, returns it's return type.
                    template <typename T> struct memptr_return_type {};
                    template <typename T, typename C> struct memptr_return_type<T (C::*)() const> {using type = T;};

                    // Checks if `zrefl_MembersHelper` is a direct member of `zrefl_MembersHelper`.
                    // ** WARNING **
                    // This check is FINICKY. It's written specifically to accept only directly defined member functions, and discard inherited ones.
                    // If you rewrite it, BE SURE TO CHECK that it didn't start to accept inherited functions.
                    // Perform following test: Define a base using `REFL_STRUCT` with `REFL_TERSE`. Define a derived class manually,
                    // then use `REFL_STRUCT` with `REFL_METADATA_ONLY` and `REFL_TERSE` on it. Make sure `Class::members_known<Derived>` returns true on both,
                    // which indicates that everything works correctly. If it breaks, `metadata_type<Derived>` specializations become ambiguous,
                    // and `members_known<Derived>` starts to return `false`. If `members_known<Base>` starts to return `false` instead, it means that your
                    // test is never able to find the function.
                    template <typename T, T> struct detect_helper {}; // No, `Meta::tag` doesn't work here.
                    template <typename T>
                    static detect_helper<typename memptr_return_type<decltype(&T::zrefl_MembersHelper)>::type (T::*)() const, &T::zrefl_MembersHelper> // Magic!
                    detect(); // Can't use a type alias here, because then Clang appears to check private-ness of `zrefl_MembersHelper` in the context of the user.

                    template <typename T>
                    static typename memptr_return_type<decltype(&T::zrefl_MembersHelper)>::type metadata_type(); // See the comment above on why we can't use a type alias here.
                };

                // Obtains the internal (member) metadata type for T, one of the two possible metadata types.
                // Should be SFINAE-friendly.
                template <typename T, typename = void> struct member_metadata {};
                template <typename T> struct member_metadata<T, Meta::void_type<decltype(member_metadata_helpers::detect<T>())>> // No, we can't use `metadata_type<T>` here. Read the comment above.
                {
                    using type = decltype(member_metadata_helpers::metadata_type<T>());
                };
                template <typename T> using member_metadata_t = typename member_metadata<T>::type;

                void zrefl_StructHelper(); // Dummy ADL target.

                // Obtains the external (non-member) metadata type for T, one of the two possible metadata types.
                // Should be SFINAE-friendly.
                template <typename T, typename = void> struct nonmember_metadata {};
                template <typename T> struct nonmember_metadata<T, Meta::void_type<decltype(zrefl_StructHelper(std::declval<const T &>()))>>
                {
                    using type = decltype(zrefl_StructHelper(std::declval<const T &>()));
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
                template <typename T> using value_explicitly_polymorphic = Meta::value_tag<T::explicitly_polymorphic>;
            }

            // All of those should be SFINAE-friendly.
            // `::value` is a `const char *` pointing to the class name.
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
            // `::value` is a `const char (*)(std::size_t index)` that returns member names based on index.
            template <typename T> using member_name = impl::metadata_type_t<impl::value_member_names_func, T>;
            // `::value` is true if the class has `REFL_POLYMORPHIC` on it.
            template <typename T> using explicitly_polymorphic = impl::metadata_type_t<impl::value_explicitly_polymorphic, T>;
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
            template <typename T> struct name<T, Meta::void_type<decltype(std::tuple_size<T>::value)>> // Tuple interface. Note that we don't use `tuple_size_v` here because it's not SFINAE-friendly.
            {
                static constexpr const char *value = "tuple";
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
                template <std::size_t I> static constexpr auto &at(T &object)
                {
                    return object.*Meta::list_value_at<Macro::member_ptrs<T>, I>;
                }
            };
            template <typename T> struct members<T, Meta::void_type<decltype(std::tuple_size<T>::value)>> // Tuple interface. Note that we don't use `tuple_size_v` here because it's not SFINAE-friendly.
            {
                static constexpr std::size_t count = std::tuple_size_v<T>;
                template <std::size_t I> static constexpr auto &at(T &object)
                {
                    return std::get<I>(object);
                }
            };
            template <typename M, std::size_t N> struct members<M[N]> // Array interface.
            {
                using T = M[N];
                static constexpr std::size_t count = N;
                template <std::size_t I> static constexpr auto &at(T &object)
                {
                    return object[I];
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
                using at = typename Meta::list_type_at<Macro::member_attribs<T>, impl::MemberIndexToAttrPackIndex(Macro::member_attribs<T>{}, I)>::list;
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
                static constexpr const char *at(std::size_t index)
                {
                    // `index` is guaranteed to be valid.
                    return Macro::member_name<T>::value(index);
                }
            };

            // Provides information on whether or not a class is explicitly declared as polymorphic (normally using `REFL_POLYMORPHIC`).
            template <typename T, typename Void = void> struct explicitly_polymorphic
            {
                static_assert(std::is_void_v<Void>);
                static constexpr bool value = false;
            };
            template <typename T> struct explicitly_polymorphic<T, Meta::void_type<Macro::explicitly_polymorphic<T>>>
            {
                static constexpr bool value = Macro::explicitly_polymorphic<T>::value;
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
        template <typename T> using direct_virtual_bases = typename Custom::virt_bases<std::remove_const_t<T>>::type;

        // Non-staic members.
        template <typename T> inline constexpr bool members_known = Custom::members<std::remove_const_t<T>>::count != std::size_t(-1);
        template <typename T> inline constexpr std::size_t member_count = members_known<T> ? Custom::members<std::remove_const_t<T>>::count : 0;
        template <std::size_t I, Meta::deduce..., typename T> constexpr decltype(auto) Member(T &&object)
        {
            using class_t = std::remove_cvref_t<T>;
            auto &ref = Custom::members<class_t>::template at<I>(const_cast<class_t &>(object));
            using member_t = std::remove_reference_t<decltype(ref)>;
            using member_cv_t = std::conditional_t<std::is_const_v<std::remove_reference_t<T>>, const member_t, member_t>;
            using member_cvref_t = std::conditional_t<std::is_reference_v<T>, member_cv_t &, member_cv_t &&>;
            return static_cast<member_cvref_t>(ref);
        }

        // Member attributes.
        // Returns a `Meta::type_list`.
        template <typename T, std::size_t I> using member_attribs = typename Custom::member_attribs<std::remove_const_t<T>>::template at<I>;

        // Member names.
        template <typename T> inline constexpr bool member_names_known = Custom::member_names<std::remove_const_t<T>>::known;
        template <typename T> [[nodiscard]] constexpr const char *MemberName(std::size_t i)
        {
            if (i >= member_count<T>)
                return nullptr;
            return Custom::member_names<std::remove_const_t<T>>::at(i);
        }

        // Check for presence of `REFL_POLYMORPHIC` on a class.
        template <typename T> inline constexpr bool explicitly_polymorphic = Custom::explicitly_polymorphic<std::remove_const_t<T>>::value;


        // This is the higher-level interface.

        // Returns the type of a member variable.
        template <typename T, std::size_t I> using member_type = std::remove_reference_t<decltype(Class::Member<I>(std::declval<T &>()))>; // Using a qualified call to protect against accidental ADL.

        namespace impl
        {
            // Helpers for recursively determining all virtual base classes.
            template <typename Bases, typename Out> struct rec_virt_bases_norm {}; // Collect all recursive virtual bases of `Bases` to `Out`.
            template <typename VirtBases, typename Out> struct rec_virt_bases_virt {}; // Collect `VirtBases` and all their recursive virtual bases to `Out`.

            template <typename Out> struct rec_virt_bases_norm<Meta::type_list<>, Out> {using type = Out;};
            template <typename FirstBase, typename ...Bases, typename Out> struct rec_virt_bases_norm<Meta::type_list<FirstBase, Bases...>, Out>
            {
                using type =
                    typename rec_virt_bases_virt<direct_virtual_bases<FirstBase>, // ^ 3. Recursively process virtual bases of the first base.
                    typename rec_virt_bases_norm<bases<FirstBase>,                // | 2. Recursively process bases of the first base.
                    typename rec_virt_bases_norm<Meta::type_list<Bases...>,       // | 1. Process the remaining bases.
                Out>::type>::type>::type;
            };

            template <typename Out> struct rec_virt_bases_virt<Meta::type_list<>, Out> {using type = Out;};
            template <typename FirstVirtBase, typename ...VirtBases, typename Out> struct rec_virt_bases_virt<Meta::type_list<FirstVirtBase, VirtBases...>, Out>
            {
                using type =
                    typename rec_virt_bases_virt<direct_virtual_bases<FirstVirtBase>, // ^ 4. Recursively process virtual bases of the first base.
                    typename rec_virt_bases_norm<bases<FirstVirtBase>,                // | 3. Recursively process bases of the first base.
                    typename rec_virt_bases_virt<Meta::type_list<VirtBases...>,       // | 2. Process the remaining virtual bases. // Sic! Note `_virt` here. The `rec_virt_bases_norm` uses `_norm` here instead.
                    Meta::list_copy_uniq<Meta::type_list<FirstVirtBase>,              // | 1. Add the first virtual base.
                Out>>::type>::type>::type;
            };
        }

        // Recursively get a list of all virtual bases of a class.
        template <typename T> using virtual_bases = typename impl::rec_virt_bases_virt<direct_virtual_bases<T>, typename impl::rec_virt_bases_norm<bases<T>, Meta::type_list<>>::type>::type;
        // A list of all bases of a class: regular ones, then virtual ones.
        // Duplicates are not removed from this list, so you should probably check for them separately. If you use `CombinedBaseIndex`, it takes care of that.
        template <typename T> using combined_bases = Meta::list_cat<bases<T>, virtual_bases<T>>;

        // Check if a class has a specific attribute.
        template <typename T, typename A> inline constexpr bool class_has_attrib = Meta::list_contains_type<class_attribs<T>, A>;
        // Check if a class member has a specific attribute.
        template <typename T, std::size_t I, typename A> inline constexpr bool member_has_attrib = Meta::list_contains_type<member_attribs<T, I>, A>;

        namespace impl
        {
            // Helper functions for `Utils::GetStringIndex`.
            // Note that even though those would be able to handle T being const, it would cause two identical
            // instantiatons of `EntryIndex` being created (for const and non-const T), which is not good.
            // Because of that we force T to not be cosnt.
            template <typename T> constexpr auto StringList_Members()
            {
                static_assert(!std::is_const_v<T>);
                std::array<const char *, member_count<T>> ret{};
                for (std::size_t i = 0; i < ret.size(); i++)
                    ret[i] = Class::MemberName<T>(i); // Using a qualified call to protect against accidental ADL.
                return ret;
            }
            template <typename L> constexpr auto StringList_Classes() // `L` is a `Meta::type_list` of classes.
            {
                if constexpr (Meta::list_size<L> == 0)
                {
                    return std::array<const char *, 0>{};
                }
                else
                {
                    return Meta::cexpr_generate_array<Meta::list_size<L>>([](auto index)
                    {
                        using type = Meta::list_type_at<L, index.value>;
                        static_assert(Class::name_known<type>, "Name of this class is not known.");
                        return name<type>;
                    });
                }
            }
        }

        // Those convert {member|base|virtual base} name to its index.
        // If there is no such entry, -1 is returned.
        // If a class has several entries with the same name, using corresponding function will cause an assertion at program startup.
        // (When we get C++20 constexpr algorithms, we can move that assertion to compile-time).
        template <typename T> [[nodiscard]] std::size_t MemberIndex(const char *name)
        {
            // Note that `remove_const_t` is necessary here, but not in the other three functions.
            return Utils::GetStringIndex<impl::StringList_Members<std::remove_const_t<T>>>(name);
        }
        template <typename T> [[nodiscard]] std::size_t BaseIndex(const char *name)
        {
            return Utils::GetStringIndex<impl::StringList_Classes<bases<T>>>(name);
        }
        template <typename T> [[nodiscard]] std::size_t VirtualBaseIndex(const char *name)
        {
            return Utils::GetStringIndex<impl::StringList_Classes<virtual_bases<T>>>(name);
        }
        template <typename T> [[nodiscard]] std::size_t CombinedBaseIndex(const char *name)
        {
            // Concatenates `bases<T>` and `virtual_bases<T>` and returns the index in the combined list.
            return Utils::GetStringIndex<impl::StringList_Classes<Meta::list_cat<bases<T>, virtual_bases<T>>>>(name);
        }

        template <typename T> [[nodiscard]] std::size_t MemberIndex      (const std::string &name) {return MemberIndex      <T>(name.c_str());}
        template <typename T> [[nodiscard]] std::size_t BaseIndex        (const std::string &name) {return BaseIndex        <T>(name.c_str());}
        template <typename T> [[nodiscard]] std::size_t VirtualBaseIndex (const std::string &name) {return VirtualBaseIndex <T>(name.c_str());}
        template <typename T> [[nodiscard]] std::size_t CombinedBaseIndex(const std::string &name) {return CombinedBaseIndex<T>(name.c_str());}
    }

    namespace Polymorphic::impl
    {
        // Attachement point for polymorphic stuff.
        // Does nothing unless `reflection/poly_storage_support.h` is included.
        template <typename T, decltype(nullptr) = nullptr> struct RegisterType {};
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


--- REFL_SIMPLE_STRUCT ---
You can use an alternative shorter notation to create reflected structures:

    REFL_SIMPLE_STRUCT( <name> <members> )

This is exactly equivalent to:

    struct <name> { REFL_MEMBERS( <members> ) };

which means that in addition to the structure name, <name> can contain base classes and other things.
<members> can be empty.

Additionally, there is `REFL_SIMPLE_STRUCT_WITHOUT_NAMES`, which
does the same thing, but uses `REFL_UNNAMED_MEMBERS`.


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
  If you want to inherit from a class virtually, write it as `virtual( <name> )`.
  Adding access specifiers is not allowed, so only public inheritance is supported.
  Can be used at most once per `REFL_STRUCT`.

* `REFL_SILENTLY_EXTENDS <base_class_list>`
  Specifies additional base classes that shouldn't be visible to reflection.
  <base_class_list> is a comma-separated list of one or more classes that
  can include access specifiers and `virtual`.
  Can be used at most once per `REFL_STRUCT`.

* `REFL_ASSUME_EXTENDS <base_classes>`
  Same as `REFL_EXTENDS`, but the base classes are only added to the metadata.
  They are not actually inherited from. This is good for when you inherit from
  those classes indirectly, in a way that's otherwise not recorded in the metadata.

* `REFL_PARAM( <pseudo-type> ) <name> [, <init>]`
  Specifies a template parameter for the structure.
  If used at least once, the structure is a template.
  <pseudo-type> is either a type of a non-type parameter,
  or `[template< ... >] {typename|class}` for a type parameter.
  Can be used more than once per `REFL_STRUCT`.

* `REFL_POLYMORPHIC`
  Specifies that the structure is polymorphic, and has a virtual destructor.
  It's achieved by inheriting from `Meta::with_virtual_destructor<T>`.
  If `reflection/poly_storage_support.h` is included, this structure
  and the ones derived from it get additional capabilities.
  Can be used at most once per `REFL_STRUCT`.

* `REFL_FINAL`
  Specifies that the structure is `final`.
  Can be used at most once per `REFL_STRUCT`.

* `REFL_METADATA_ONLY`
  Prevents structure definition from being generated, but the metadata
  is still generated. See the next section.

* `REFL_AT_CLASS_SCOPE`
  Must be used if and only if the structure is defined inside of an another structure.
  Failure to do so causes a compile-time error.

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

External metadata, at the same scope as the class.

    struct <metadata_name>
    {
        // Name.
        static constexpr char name[] =  "StructName";

        // Class attributes. Can be absent if there are none.
        using attribs = Refl::impl::Class::ClassAttr<ATTRIBS>;

        // A list of bases.
        using bases = ::Meta::type_list<base1, base2, ...>;
        // A list of virtual bases.
        using virt_bases = ::Meta::type_list<base3, base4, ...>;

        // Should be present only if true.
        // Indicates that the class was explicitly made polymorphic using `REFL_POLYMORPHIC`.
        static constexpr bool explicitly_polymorphic() {return true;}

        // If both `REFL_METADATA_ONLY` and `REFL_TERSE` are used,
        // members metadata will also be here.
        // It's will look exactly like the contents of `struct Helper` described below.
    };
    [[maybe_unused]] {inline|friend} <metadata_name> zrefl_StructHelper(const A &)
    {
        return {};
    }

Members metadata, inside of a class.

    struct <metadata_name>
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
    <metadata_name> zrefl_MembersHelper() const
    {
        return {};
    }

    friend struct Refl::Class::Macro::impl::member_metadata_helpers;

    void zrefl_RegistrationHelper()
    {
        [[maybe_unused]] Refl::Polymorphic::impl::RegisterType<std::remove_reference_t<decltype(*this)>>
            zrefl_RegistrationHelperVar;
    }

That's all.
*/


// An optional parameter for `REFL_STRUCT`.
// Specifies a list of base classes for the struct.
// Adding access specifiers is not allowed, only public inheritance is supported.
// To inherit from a class virtually, write it as `virtual(ClassName)`. Note that this
// syntax was carefully chosen: while the list of regular bases can be copied verbatim
// to the metadata, the list of virtual bases needs to be written without `virtual` in
// the metadata, and with `virtual` in the struct definition. We can't simply have a
// separate comma-separated list of virtual bases, since their names can contain commas
// in template parameter lists.
#define REFL_EXTENDS MA_PARAM(ReflBases)
#define MA_PARAMS_category_ReflStruct_X_ReflBases
#define MA_PARAMS_equal_ReflBases_X_ReflBases

// An optional parameter for `REFL_STRUCT`.
// Specifies a list of untracked base classes for the struct.
// Those bases will be invisible for the reflection (will not be added to metadata).
// You can freely put access specifiers and `virtual` on those.
#define REFL_SILENTLY_EXTENDS MA_PARAM(ReflUntrackedBases)
#define MA_PARAMS_category_ReflStruct_X_ReflUntrackedBases
#define MA_PARAMS_equal_ReflUntrackedBases_X_ReflUntrackedBases

// An optional parameter for `REFL_STRUCT`.
// Specifies a list of base classes for the struct that should be added
// to the metadata, but not actually inherited from. Good for when you
// indirectly inherit from those classes in a way that can't normally be detected.
// Uses the same syntax as `REFL_EXTENDS`.
#define REFL_ASSUME_EXTENDS MA_PARAM(ReflFakeBases)
#define MA_PARAMS_category_ReflStruct_X_ReflFakeBases
#define MA_PARAMS_equal_ReflFakeBases_X_ReflFakeBases

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
// This must be present if and only if the struct is at class scope.
// Failure to do it causes a hard error.
// We define it conditionally, because the same macro is used for enums.
#ifndef REFL_AT_CLASS_SCOPE
#define REFL_AT_CLASS_SCOPE MA_PARAM(ReflAtClassScope)
#define MA_PARAMS_equal_ReflAtClassScope_X_ReflAtClassScope
#endif
#define MA_PARAMS_category_ReflStruct_X_ReflAtClassScope

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
        (MA_PARAMS_GET(, ReflStruct, ReflBases, seq, MA_PARAMS_IDENTITY)), \
        (MA_PARAMS_GET(, ReflStruct, ReflFakeBases, seq, MA_PARAMS_IDENTITY)), \
        (MA_PARAMS_GET_ONE(, ReflStruct, ReflUntrackedBases, seq, MA_PARAMS_IDENTITY)), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflIsPoly, seq, MA_PARAMS_DUMMY_EMPTY), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflIsFinal, seq, MA_PARAMS_DUMMY_EMPTY), \
        (MA_PARAMS_GET_ONE(, ReflStruct, ReflAttr, seq, MA_PARAMS_IDENTITY)), \
        __VA_OPT__(REFL_STRUCT_impl_get_body(__VA_ARGS__)), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflMetadataOnly, seq, MA_PARAMS_DUMMY_EMPTY), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflAtClassScope, seq, MA_PARAMS_DUMMY_EMPTY) \
    )

// Internal. Helper for `REFL_STRUCT_impl`.
// Given `body,`, returns `body`. Emits an error if there is more than one comma.
#define REFL_STRUCT_impl_get_body(body, ...) \
    __VA_OPT__(MA_ABORT("Invalid usage of REFL_STRUCT(...).")) body

// Internal. Declares a reflected struct, and a metadata for it.
// Also generates a struct body if `REFL_TERSE` is present, but `REFL_METADATA_ONLY` is not.
// `name_` is a class name. `tparams_seq_` is a sequence of template parameters: `(type,name[,init])...` or empty,
// `*bases_` are lists of base classes: `(base1,base2,...)` or `()` if empty, `*_if_not_empty_` are equal to `x` or empty,
// `body_or_empty_` is `(unnamed_if_not_empty_,seq_)` or empty.
#define REFL_STRUCT_impl_low(name_, tparams_seq_, bases_, fake_bases_, untracked_bases_, is_poly_if_not_empty_, is_final_if_not_empty_, attribs_, body_or_empty_, metadata_only_if_not_empty_, class_scope_if_not_empty_) \
    /* Make sure we know what scope we're at. */\
    /* This causes a hard error if we're at class scope, but there is no `REFL_AT_CLASS_SCOPE`. */\
    /* We don't need to check the opposite scenario here, because we use a friend function in that case. */\
    MA_IF_NOT_EMPTY_ELSE(, namespace {}, class_scope_if_not_empty_) \
    /* Unless we're generating metadata only, declare the structure. */\
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_decl, metadata_only_if_not_empty_) \
        (name_, tparams_seq_) \
    /* Define the primary metadata structure. */\
    /* Note that we can't define this struct inside of the function below. */\
    /* It for some reason prevents metadata from being accessible in */\
    /* enclosing class, if this class is defined at class scope. */\
    /* If this is a template, and we're in the metadata-only mode, this has to be a template as well. */\
    MA_IF_NOT_EMPTY(REFL_STRUCT_impl_tparams_decl(tparams_seq_), metadata_only_if_not_empty_) \
    struct MA_CAT(zrefl_StructHelper_, name_) \
    { \
        /* A name. */\
        /* (Note that an array is used instead of a pointer. A pointer couldn't be put into */\
        /* a `Meta::value_tag`, so we'd need to change the implementation of `Refl::Class::Macro::name`.) */\
        static constexpr char name[] = MA_STR(name_); \
        /* Attribute list. */\
        MA_IF_NOT_EMPTY(using attribs = ::Refl::impl::Class::ClassAttr<MA_IDENTITY attribs_>;, MA_IDENTITY attribs_) \
        /* A list of bases. */\
        using bases = ::Meta::type_list<REFL_STRUCT_impl_strip_leading_comma(REFL_STRUCT_impl_nonvirt_bases(MA_TR_C(MA_IDENTITY bases_) MA_TR_C(MA_IDENTITY fake_bases_)))>; \
        /* A list of virtual bases. */\
        using virt_bases = ::Meta::type_list<REFL_STRUCT_impl_strip_leading_comma(REFL_STRUCT_impl_virt_bases_with_prefix(,MA_TR_C(MA_IDENTITY bases_) MA_TR_C(MA_IDENTITY fake_bases_)))>; \
        /* Indicates that the class was explicitly made polymorphic. */\
        MA_IF_NOT_EMPTY(static constexpr bool explicitly_polymorphic = true;, is_poly_if_not_empty_) \
        /* If both `REFL_METADATA_ONLY` and `REFL_TERSE` are used, generate metadata */\
        /* for member variables here instead of its normal location. */\
        MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_extra_metadata, MA_INVERT_EMPTINESS(body_or_empty_) MA_INVERT_EMPTINESS(metadata_only_if_not_empty_)) \
            (name_ REFL_STRUCT_impl_tparams(tparams_seq_), MA_IDENTITY body_or_empty_) \
    }; \
    /* Define a helper function that returns the metadata structure. */\
    REFL_STRUCT_impl_tparams_decl(tparams_seq_) /* Template parameters. */ \
    [[maybe_unused]] MA_IF_NOT_EMPTY_ELSE(friend, inline, class_scope_if_not_empty_) \
    MA_CAT(zrefl_StructHelper_, name_) /* Return type. */\
    MA_IF_NOT_EMPTY(REFL_STRUCT_impl_tparams(tparams_seq_), metadata_only_if_not_empty_) /* Template parameters of the return type. Those are only needed if it's a template and we're in metadata-only mode. */\
    zrefl_StructHelper(const name_ REFL_STRUCT_impl_tparams(tparams_seq_) &) {return {};} \
    /* Generate the beginning of the definition of the structure. */\
    /* It includes the struct name and a list of bases. */\
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_header, metadata_only_if_not_empty_) \
        (name_, tparams_seq_, bases_, untracked_bases_, is_poly_if_not_empty_, is_final_if_not_empty_) \
    /* If `REFL_TERSE` is used (and `REFL_METADATA_ONLY` is not), generate a simple body for the structure. */\
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_body, MA_INVERT_EMPTINESS(body_or_empty_) metadata_only_if_not_empty_) \
        (MA_IDENTITY body_or_empty_) \
    /* If both `REFL_METADATA_ONLY` and `REFL_TERSE` are used, and this is not a template, */\
    /* generate struct registration helper here, instead of it's normal location. */\
    /* See `zrefl_RegistrationHelper` for the explanation on why we don't register templates. */\
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_registration_helper, MA_INVERT_EMPTINESS(body_or_empty_) MA_INVERT_EMPTINESS(metadata_only_if_not_empty_) tparams_seq_)(name_) \

// Internal. Helper for `REFL_STRUCT_impl_low`. Declares a structure, possibly with template parameters.
#define REFL_STRUCT_impl_low_decl(name_, tparams_seq_) \
    REFL_STRUCT_impl_tparams_firstdecl(tparams_seq_) struct name_;

// Internal. Helper for `REFL_STRUCT_impl_low`. Generates the beginning of the definition of a structure,
// which includes its name and base classes.
#define REFL_STRUCT_impl_low_header(name_, tparams_seq_, bases_, untracked_bases_, is_poly_if_not_empty_, is_final_if_not_empty_) \
    REFL_STRUCT_impl_tparams_decl(tparams_seq_) struct name_ \
    MA_IF_NOT_EMPTY(final, is_final_if_not_empty_) \
    MA_IF_NOT_EMPTY_ELSE(REFL_STRUCT_impl_low_expand_bases, MA_NULL, MA_IDENTITY bases_ MA_IDENTITY untracked_bases_ is_poly_if_not_empty_) \
    ( \
        name_ REFL_STRUCT_impl_tparams(tparams_seq_), \
        bases_, untracked_bases_, \
        is_poly_if_not_empty_ \
    )

// Internal. Helper for `REFL_STRUCT_impl_low`. Generates a list of base classes, starting with a colon.
// `name` is the struct name, possibly followed by `<...>` template parameters.
// `*bases` are lists of base classes: `(class1,class2,...)` or `()` if empty.
// `...` is `x` if the class is polymorphic, empty otherwise.
#define REFL_STRUCT_impl_low_expand_bases(name, bases, untracked_bases, /*is_poly_if_not_empty*/...) \
    : REFL_STRUCT_impl_strip_leading_comma( \
        /* Regular bases. */\
        REFL_STRUCT_impl_nonvirt_bases(MA_TR_C(MA_IDENTITY bases)) \
        /* Virtual bases. */\
        REFL_STRUCT_impl_virt_bases_with_prefix(virtual, MA_TR_C(MA_IDENTITY bases)) \
        /* Untracked bases. */\
        MA_LE_C(MA_IDENTITY untracked_bases) \
        /* The special base that makes the struct polymorphic if needed. */\
        __VA_OPT__(,::Meta::with_virtual_destructor<name>) \
    )

#define REFL_STRUCT_impl_strip_leading_comma(...) __VA_OPT__(REFL_STRUCT_impl_strip_leading_comma_low(__VA_ARGS__))
#define REFL_STRUCT_impl_strip_leading_comma_low(x, ...) __VA_ARGS__

// Internal. Helper for `REFL_STRUCT_impl_low`. Generates a simple body for the structure (for `REFL_TERSE`).
#define REFL_STRUCT_impl_low_body(...) REFL_STRUCT_impl_low_body_low(__VA_ARGS__)
#define REFL_STRUCT_impl_low_body_low(unnamed_if_not_empty, seq) \
    { MA_IF_NOT_EMPTY_ELSE(REFL_UNNAMED_MEMBERS_impl, REFL_MEMBERS_impl, unnamed_if_not_empty)(seq) };

// Internal. Helper for `REFL_STRUCT_impl_low`. Used for generating field metadata if both `REFL_METADATA_ONLY` and `REFL_TERSE` are used.
#define REFL_STRUCT_impl_low_extra_metadata(...) REFL_STRUCT_impl_low_extra_metadata_low(__VA_ARGS__)
#define REFL_STRUCT_impl_low_extra_metadata_low(name, unnamed_if_not_empty, seq) \
    using t [[maybe_unused]] = name; \
    REFL_MEMBERS_impl_metadata_generic(seq) \
    MA_IF_NOT_EMPTY_ELSE(MA_NULL, REFL_STRUCT_impl_low_extra_metadata_low_names, unnamed_if_not_empty)(seq)

#define REFL_STRUCT_impl_low_extra_metadata_low_names(seq) REFL_MEMBERS_impl_metadata_memname(seq)


// Internal. Helper for `REFL_STRUCT_impl_low`. Generates a class registration helper. It's generated here only if both `REFL_METADATA_ONLY` and `REFL_TERSE` are used.
// `name_` is the struct name. Note that at this point it can't be a template.
#define REFL_STRUCT_impl_low_registration_helper(name_) REFL_MEMBERS_impl_metadata_registration_helper(name_, inline static)

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

// Internal. Helper for `REFL_STRUCT_impl_low`. Extracts non-virtual bases from a list of bases.
// `...` is a comma-separated list of bases (with trailing comma), where virtual bases are written as `virtual(ClassName)`.
// Returns a comma-separated list with a leading (sic) comma.
#define REFL_STRUCT_impl_nonvirt_bases(...) \
    MA_VA_FOR_EACH(, REFL_STRUCT_impl_nonvirt_bases_low, __VA_ARGS__)
#define REFL_STRUCT_impl_nonvirt_bases_low(data, index, elem) \
    MA_IF_NOT_EMPTY(MA_COMMA() elem, MA_CAT(REFL_STRUCT_impl_virtual_null_, elem))

// Internal. Helper for `REFL_STRUCT_impl_low`. Extracts virtual bases from a list of bases, possibly adds a prefix to each.
// `...` is a comma-separated list of bases (with trailing comma), where virtual bases are written as `virtual(ClassName)`.
// Returns a comma-separated list with a leading (sic) comma.
#define REFL_STRUCT_impl_virt_bases_with_prefix(maybe_virtual, ...) \
    MA_VA_FOR_EACH(maybe_virtual, REFL_STRUCT_impl_virt_bases_with_prefix_low, __VA_ARGS__)
#define REFL_STRUCT_impl_virt_bases_with_prefix_low(maybe_virtual, index, elem) \
    MA_IF_NOT_EMPTY(MA_COMMA() maybe_virtual MA_CAT(REFL_STRUCT_impl_virtual_iden_, elem), MA_INVERT_EMPTINESS(MA_CAT(REFL_STRUCT_impl_virtual_null_, elem)))

// Internal. Helpers for `REFL_STRUCT_impl_[non]virt_bases[_with_prefix]`.
// The `virtual` part at the end is added with MA_CAT.
#define REFL_STRUCT_impl_virtual_null_virtual(...)
#define REFL_STRUCT_impl_virtual_iden_virtual(...) __VA_ARGS__


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
    REFL_MEMBERS_impl_meta_begin \
    REFL_MEMBERS_impl_metadata_generic(__VA_ARGS__) \
    REFL_MEMBERS_impl_metadata_memname(__VA_ARGS__) \
    REFL_MEMBERS_impl_meta_end

// Same as `REFL_MEMBERS`, but doesn't save variable names.
#define REFL_UNNAMED_MEMBERS(...) REFL_UNNAMED_MEMBERS_impl((__VA_ARGS__))
#define REFL_UNNAMED_MEMBERS_impl(...) \
    REFL_MEMBERS_impl_decl(__VA_ARGS__) \
    REFL_MEMBERS_impl_meta_begin \
    REFL_MEMBERS_impl_metadata_generic(__VA_ARGS__) \
    REFL_MEMBERS_impl_meta_end

// `REFL_SIMPLE_STRUCT( <name> <members> )` is exactly equivalent to `struct <name> { REFL_MEMBERS( <members> ) };`
#define REFL_SIMPLE_STRUCT(...) struct MA_SEQ_FIRST((__VA_ARGS__)) { REFL_MEMBERS_impl(() MA_SEQ_NO_FIRST((__VA_ARGS__)) ) };

// Same as `REFL_SIMPLE_STRUCT`, but the members are unnamed.
#define REFL_SIMPLE_STRUCT_WITHOUT_NAMES(...) struct MA_SEQ_FIRST((__VA_ARGS__)) { REFL_UNNAMED_MEMBERS_impl(() MA_SEQ_NO_FIRST((__VA_ARGS__)) ) };

// Internal. Helper for `REFL_MEMBERS`. Expands to the opening of the embedded metadata section.
#define REFL_MEMBERS_impl_meta_begin \
    /* Note that we can't define this struct inside of the function below. */\
    /* It for some reason prevents metadata from being accessible in */\
    /* enclosing class, if this class is defined at class scope. */\
    template <typename zrefl_ThisType> struct zrefl_MembersHelperType \
    { \
        using t [[maybe_unused]] = zrefl_ThisType;

// Internal. Helper for `REFL_MEMBERS`. Expands to the ending of the embedded metadata section.
#define REFL_MEMBERS_impl_meta_end \
    }; \
    auto zrefl_MembersHelper() const -> zrefl_MembersHelperType<::std::remove_cvref_t<decltype(*this)>> {return {};} \
    friend struct ::Refl::Class::Macro::impl::member_metadata_helpers; \
    /* If the class is polymorphic, register it as one.                      */\
    /* This thing is all the way down there, in a separate function, because */\
    /* it needs the class to be defined, as well as its metadata.            */\
    /* Note that since it's placed in a non-static member function,          */\
    /* the class won't be registered if it's a template, since the function  */\
    /* will never be called. This is not a problem though, since we wouldn't */\
    /* be able to diffirentiate between different instantiations of the      */\
    /* template anyway, since we don't include template arguments in class   */\
    /* names. If one day you decide that this has to change, you can         */\
    /* instantiate this helper variable in an immediately-invoked lambda     */\
    /* initializer of a non-static data member (to allow `decltype(*this)`), */\
    /* marked with `[[no_unique_address]]`. This will cause it to be         */\
    /* instantiated only if any constructor is instantiated, but it sounds   */\
    /* like an acceptable tradeoff. The only alternative I'm aware of is     */\
    /* putting it in a virtual function, since they are instantiated         */\
    /* immediately even if not used. But that would make our classes         */\
    /* unconditionally polymorphic, so it's not acceptable.                  */\
    void zrefl_RegistrationHelper() \
    { \
        REFL_MEMBERS_impl_metadata_registration_helper(::std::remove_reference_t<decltype(*this)>,) \
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
        /* can't use `MA_IDENTITY` here, since it would conflict with the same macro in `REFL_STRUCT_impl_low_base` */\
        MA_IDENTITY2( MA_NULL MA_APPEND_TO_VA_END(_end, REFL_MEMBERS_impl_metadata_memattr_loop_a __VA_ARGS__) () ) \
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
#define REFL_MEMBERS_impl_metadata_memattr_loop_a(...) REFL_MEMBERS_impl_metadata_memattr_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memattr_loop_b
#define REFL_MEMBERS_impl_metadata_memattr_loop_b(...) REFL_MEMBERS_impl_metadata_memattr_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memattr_loop_a
#define REFL_MEMBERS_impl_metadata_memattr_loop_a_end
#define REFL_MEMBERS_impl_metadata_memattr_loop_b_end

// Internal. Helper for `REFL_MEMBERS_impl_metadata_memattr_loop_*`. Called once for each `DECL_REFL` for structs that use attributes.
// Expands to `::Refl::impl::Class::Attr<count, attribs>`, where `count` is the amount of variables in this `DECL_REFL` (represented as repeated `+1`),
// and `attribs` is a list of attributes.
#define REFL_MEMBERS_impl_metadata_memattr_body(params, ...) \
    MA_IF_NOT_EMPTY_ELSE(REFL_MEMBERS_impl_metadata_memattr_body_low, MA_NULL, params)(MA_PARAMS_GET_ONE(, ReflMemberDecl, ReflAttr, params, MA_PARAMS_PARENS), __VA_ARGS__)

#define REFL_MEMBERS_impl_metadata_memattr_body_low(maybe_attr, ...) \
    (,) \
    ::Refl::impl::Class::Attr< \
        MA_VA_FOR_EACH(, REFL_MEMBERS_impl_metadata_memattr_plus1, MA_TR_C(__VA_ARGS__)), \
        ::Meta::type_list<MA_IF_NOT_EMPTY(MA_IDENTITY maybe_attr, maybe_attr)> \
    > \
    MA_IDENTITY

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


// Internal. Causes a template instantiation that registers the struct as polymorphic. (Not included in the common metadata).
// `this_type_` is either the struct name itself, or something involving `decltype(*this)`.
// `prefix_` is added to the decl-specifier-seq of the variable.
// Note that this variable has to instantiate after all other metadata has been defined. Otherwise it's not going to work.
#define REFL_MEMBERS_impl_metadata_registration_helper(this_type_, prefix_) \
    [[maybe_unused]] prefix_ ::Refl::Polymorphic::impl::RegisterType<this_type_> zrefl_RegistrationHelperVar;


#if 0 // Tests

REFL_STRUCT(MyStruct REFL_SILENTLY_EXTENDS std::vector<int> REFL_EXTENDS std::string, std::string_view, virtual(std::vector<float>))
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
