#pragma once

#include <cstddef>
#include <exception>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "utils/macro.h"
#include "utils/meta.h"
#include "utils/named_macro_parameters.h"

namespace Refl
{
    // A base class for field attributes.
    struct BasicAttribute {};

    namespace impl::Struct
    {
        template <std::size_t N, typename ...P> struct Attr
        {
            static_assert(((std::is_base_of_v<BasicAttribute, P> && !std::is_same_v<BasicAttribute, P>) && ...), "Attributes must inherit from Refl::BasicAttribute.");
        };
    }

    template <typename T>
    class Interface_Struct : public InterfaceBasic<T>
    {
      public:
        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options) const override
        {
            // output.WriteChar('[');

            // auto next_options = options;
            // if (options.multiline)
            //     next_options.extra_indent += options.indent;

            // std::size_t index = 0, size = Size(object);
            // ForEach(object, [&](const elem_t &elem)
            // {
            //     if (options.multiline)
            //         output.WriteChar('\n').WriteChar(' ', next_options.extra_indent);

            //     Interface<elem_t>().ToString(elem, output, next_options);

            //     if (index != size-1 || options.multiline)
            //         output.WriteChar(',');

            //     index++;
            // });

            // if (options.multiline)
            //     output.WriteChar('\n').WriteChar(' ', options.extra_indent);

            // output.WriteChar(']');
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

    // template <typename T>
    // struct impl::SelectInterface<T, std::enable_if_t<Meta::is_detected<impl::StdContainer::has_sane_begin_end, T> && !impl::ForceNotContainer<T>::value>>
    // {
    //     using type = Interface_StdContainer<T>;
    // };
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

* `REFL_BODY <body>`
  Causes a terse structure definition syntax to be used.
  If it's used, `REFL_STRUCT` generates a simple structure
  body (with the trailing semicolon). If you use this parameter,
  you must not specify the body manually below the macro invocation.
  <body> is a list of members, interpreted as if passed to `REFL_MEMBERS`.
  It can be used at most once per `REFL_STRUCT`.
  This parameter is special. If present, it must be the last parameter.

* `REFL_BODY_WITHOUT_NAMES`
  Same as `REFL_BODY`, but doesn't generate metadata for the field names.


--- GENERATING METADATA ---
It's possible to reflect an existing structure.
To do so, you need to generate a metadata for it.

You can generate metadata by invoking `REFL_STRUCT` with
the `REFL_METADATA_ONLY` parameter.

Generating metadata is only possible using `REFL_BODY[_WITHOUT_NAMES]`
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

        // If both `REFL_METADATA_ONLY` and `REFL_BODY` are used,
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
            // One `Refl::impl::Struct::Attr` corresponds to one `REFL_DECL`.
            using member_attribs = Meta::type_list<
                Refl::impl::Struct::Attr<COUNT1, ATTRIBS1>,
                Refl::impl::Struct::Attr<COUNT2, ATTRIBS2>
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
// `REFL_BODY_WITHOUT_NAMES` is similar to `REFL_BODY`, but uses `REFL_UNNAMED_MEMBERS`.
#define REFL_BODY )),(,(
#define REFL_BODY_WITHOUT_NAMES )),(x,(

// Declares a reflected struct, and some metadata for it.
// Must be followed by a struct body (unless `REFL_BODY` parameter is present).
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
// Also generates a struct body if `REFL_BODY` is present, but `REFL_METADATA_ONLY` is not.
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
        /* If both `REFL_METADATA_ONLY` and `REFL_BODY` are used, generate metadata */\
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
    /* If `REFL_BODY` is used (and `REFL_METADATA_ONLY` is not), generate a simple body for the structure. */\
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

// Internal. Helper for `REFL_STRUCT_impl_low`. Generates a simple body for the structure (for `REFL_BODY`).
#define REFL_STRUCT_impl_low_body(...) REFL_STRUCT_impl_low_body_low(__VA_ARGS__)
#define REFL_STRUCT_impl_low_body_low(unnamed_if_not_empty, seq) \
    { MA_IF_NOT_EMPTY_ELSE(REFL_UNNAMED_MEMBERS_impl, REFL_MEMBERS_impl, unnamed_if_not_empty)(seq) };

// Internal. Helper for `REFL_STRUCT_impl_low`. Used for generating field metadata if both `REFL_METADATA_ONLY` and `REFL_BODY` are used.
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
// Expands to `::Refl::impl::Struct::Attr<count, attribs>`, where `count` is the amount of variables in this `DECL_REFL` (represented as repeated `+1`),
// and `attribs` is a list of attributes.
#define REFL_MEMBERS_impl_metadata_memattr_body(params, ...) \
    MA_IF_NOT_EMPTY_ELSE(REFL_MEMBERS_impl_metadata_memattr_body_low, MA_NULL, params)(MA_PARAMS_GET_ONE(, ReflMemberDecl, ReflAttr, params, MA_PARAMS_PARENS), __VA_ARGS__)

#define REFL_MEMBERS_impl_metadata_memattr_body_low(maybe_attr, ...) \
    ::Refl::impl::Struct::Attr< \
        MA_VA_FOR_EACH(, REFL_MEMBERS_impl_metadata_memattr_plus1, MA_TR_C(__VA_ARGS__)) \
        MA_IF_NOT_EMPTY(MA_COMMA() MA_IDENTITY maybe_attr, maybe_attr) \
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


#if 1 // Tests

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

REFL_STRUCT(B)
{
    REFL_MEMBERS(
        REFL_DECL(int REFL_INIT =0) x, y, z
        REFL_DECL(float) w, ww
        REFL_DECL(float REFL_ATTR int) h, hh
    )
};

REFL_STRUCT( C
    REFL_BODY
    REFL_DECL(int) x,y
    REFL_DECL(float) z
)

REFL_STRUCT( D
    REFL_BODY_WITHOUT_NAMES
    REFL_DECL(int) x,y
    REFL_DECL(float) z
)

struct E {int x, y; float z;};

REFL_STRUCT( E
    REFL_METADATA_ONLY
    REFL_BODY_WITHOUT_NAMES
    REFL_DECL() x,y
    REFL_DECL() z
)

#endif
