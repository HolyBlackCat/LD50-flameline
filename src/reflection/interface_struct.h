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
// Must be followed by `Type , Name [, Init]`.
#define REFL_PARAM MA_PARAM(ReflTemplateParam)
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

// Declares a reflected struct, and some metadata for it.
// Must be followed by the struct body.
#define REFL_STRUCT(...) \
    REFL_STRUCT_impl(((__VA_ARGS__)))

// Internal. Declares a reflected struct.
#define REFL_STRUCT_impl(seq) \
    REFL_STRUCT_impl_low( \
        MA_PARAMS_FIRST(seq), \
        MA_PARAMS_GET(, ReflStruct, ReflTemplateParam, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET(, ReflStruct, ReflBase, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET(, ReflStruct, ReflVirtualBase, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflUntrackedBases, seq, MA_PARAMS_PARENS), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflIsFinal, seq, MA_EMPTY_COMMA), \
        MA_PARAMS_GET_ONE(, ReflStruct, ReflIsPoly, seq, MA_EMPTY_COMMA) \
    )

// Internal. Declares a reflected struct.
#define REFL_STRUCT_impl_low(name_, tparams_, base_seq_, virt_base_seq_, untracked_bases_, is_final_if_not_empty_, is_poly_if_not_empty_) \
    MA_IF_NOT_EMPTY(template <REFL_STRUCT_impl_tparams_longdecl(tparams_)>, tparams_) \
    struct name_; \
    struct MA_CAT(zrefl_StructHelper_, name_) \
    { \
        using bases = ::Meta::type_list<MA_SEQ_TO_VA(base_seq_)>; \
        using virt_bases = ::Meta::type_list<MA_SEQ_TO_VA(virt_base_seq_)>; \
    }; \
    MA_IF_NOT_EMPTY(template <REFL_STRUCT_impl_tparams_decl(tparams_)>, tparams_) \
    [[maybe_unused]] inline static MA_CAT(zrefl_StructHelper_, name_) zrefl_StructFunc(::Meta::tag<name_ MA_IF_NOT_EMPTY(<REFL_STRUCT_impl_tparams(tparams_)>, tparams_)>) {return {};} \
    MA_IF_NOT_EMPTY(template <REFL_STRUCT_impl_tparams_decl(tparams_)>, tparams_) \
    struct name_ \
    MA_IF_NOT_EMPTY(final, is_final_if_not_empty_) \
    MA_IF_NOT_EMPTY_ELSE(REFL_STRUCT_impl_low_expand_bases, MA_NULL, base_seq_ virt_base_seq_ untracked_bases_ is_poly_if_not_empty_) \
    ( \
        name_ MA_IF_NOT_EMPTY(<REFL_STRUCT_impl_tparams(tparams_)>, tparams_), \
        base_seq_, virt_base_seq_, untracked_bases_, \
        is_poly_if_not_empty_ \
    )

// Internal. Expands a sequence of template parameters `(type,name[,init])...` to a list of parameter declarations with default values: `type name = init, ...`.
#define REFL_STRUCT_impl_tparams_longdecl(seq) MA_APPEND_TO_VA_END(_end, REFL_STRUCT_impl_tparams_longdecl_loop_0 seq)
#define REFL_STRUCT_impl_tparams_longdecl_loop_0(...)   REFL_STRUCT_impl_tparams_longdecl_loop_body(__VA_ARGS__) REFL_STRUCT_impl_tparams_longdecl_loop_a
#define REFL_STRUCT_impl_tparams_longdecl_loop_a(...) , REFL_STRUCT_impl_tparams_longdecl_loop_body(__VA_ARGS__) REFL_STRUCT_impl_tparams_longdecl_loop_b
#define REFL_STRUCT_impl_tparams_longdecl_loop_b(...) , REFL_STRUCT_impl_tparams_longdecl_loop_body(__VA_ARGS__) REFL_STRUCT_impl_tparams_longdecl_loop_a
#define REFL_STRUCT_impl_tparams_longdecl_loop_0_end
#define REFL_STRUCT_impl_tparams_longdecl_loop_a_end
#define REFL_STRUCT_impl_tparams_longdecl_loop_b_end
#define REFL_STRUCT_impl_tparams_longdecl_loop_body(type, ...) MA_IF_COMMA_ELSE(REFL_STRUCT_impl_tparams_longdecl_loop_body3,REFL_STRUCT_impl_tparams_longdecl_loop_body2,__VA_ARGS__)(type, __VA_ARGS__)
#define REFL_STRUCT_impl_tparams_longdecl_loop_body3(type, name, /*init*/...) type name = __VA_ARGS__
#define REFL_STRUCT_impl_tparams_longdecl_loop_body2(type, name) type name

// Internal. Expands a sequence of template parameters `(type,name[,init])...` to a list of parameter declarations: `type name, ...`.
#define REFL_STRUCT_impl_tparams_decl(seq) MA_APPEND_TO_VA_END(_end, REFL_STRUCT_impl_tparams_decl_loop_0 seq)
#define REFL_STRUCT_impl_tparams_decl_loop_0(...)   REFL_STRUCT_impl_tparams_decl_loop_body(__VA_ARGS__,) REFL_STRUCT_impl_tparams_decl_loop_a
#define REFL_STRUCT_impl_tparams_decl_loop_a(...) , REFL_STRUCT_impl_tparams_decl_loop_body(__VA_ARGS__,) REFL_STRUCT_impl_tparams_decl_loop_b
#define REFL_STRUCT_impl_tparams_decl_loop_b(...) , REFL_STRUCT_impl_tparams_decl_loop_body(__VA_ARGS__,) REFL_STRUCT_impl_tparams_decl_loop_a
#define REFL_STRUCT_impl_tparams_decl_loop_0_end
#define REFL_STRUCT_impl_tparams_decl_loop_a_end
#define REFL_STRUCT_impl_tparams_decl_loop_b_end
#define REFL_STRUCT_impl_tparams_decl_loop_body(type, name, ...) type name

// Internal. Expands a sequence of template parameters `(type,name[,init])...` to a comma-separated list of parameter names.
#define REFL_STRUCT_impl_tparams(seq) MA_APPEND_TO_VA_END(_end, REFL_STRUCT_impl_tparams_loop_0 seq)
#define REFL_STRUCT_impl_tparams_loop_0(...)   REFL_STRUCT_impl_tparams_loop_body(__VA_ARGS__,) REFL_STRUCT_impl_tparams_loop_a
#define REFL_STRUCT_impl_tparams_loop_a(...) , REFL_STRUCT_impl_tparams_loop_body(__VA_ARGS__,) REFL_STRUCT_impl_tparams_loop_b
#define REFL_STRUCT_impl_tparams_loop_b(...) , REFL_STRUCT_impl_tparams_loop_body(__VA_ARGS__,) REFL_STRUCT_impl_tparams_loop_a
#define REFL_STRUCT_impl_tparams_loop_0_end
#define REFL_STRUCT_impl_tparams_loop_a_end
#define REFL_STRUCT_impl_tparams_loop_b_end
#define REFL_STRUCT_impl_tparams_loop_body(type, name, ...) name


// Internal. Expands to a list of base classes of a struct.
#define REFL_STRUCT_impl_low_expand_bases(name, base_seq, virt_base_seq, untracked_bases_, /*is_poly_if_not_empty*/...) \
    : MA_IDENTITY2( /* can't use `MA_IDENTITY` here, since it would conflict with the same macro in `REFL_STRUCT_impl_low_base` */\
        MA_NULL \
        MA_SEQ_FOR_EACH(, REFL_STRUCT_impl_low_base, __VA_OPT__((::Meta::with_virtual_destructor<name>)) base_seq) \
        MA_SEQ_FOR_EACH(virtual, REFL_STRUCT_impl_low_base, virt_base_seq) \
        MA_IF_NOT_EMPTY(REFL_STRUCT_impl_low_base(,,MA_IDENTITY untracked_bases_), untracked_bases_) \
        () \
    )

// Internal. Expands to a single base class of a struct.
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
#define REFL_MEMBERS(...) \
    REFL_MEMBERS_impl_decl((__VA_ARGS__)) \
    auto zrefl_MembersHelper() const \
    { \
        using t [[maybe_unused]] = std::remove_cv_t<std::remove_pointer_t<decltype(this)>>; \
        struct Helper \
        { \
            REFL_MEMBERS_impl_metadata_generic((__VA_ARGS__)) \
            REFL_MEMBERS_impl_metadata_memname((__VA_ARGS__)) \
        }; \
        return Helper{}; \
    }

// Same as `REFL_MEMBERS`, but doesn't save variable names.
#define REFL_MEMBERS_NAMELESS(...) \
    REFL_MEMBERS_impl_decl((__VA_ARGS__)) \
    auto zrefl_MembersHelper() const \
    { \
        using t = std::remove_cv_t<std::remove_pointer_t<decltype(this)>>; \
        struct Helper \
        { \
            REFL_MEMBERS_impl_metadata_generic((__VA_ARGS__)) \
        }; \
        return Helper{}; \
    }


// Internal. Declares several member variables of a struct, without any metadata.
#define REFL_MEMBERS_impl_decl(...) REFL_MEMBERS_impl_decl_low(REFL_MEMBERS_impl_skip_first __VA_ARGS__)
#define REFL_MEMBERS_impl_skip_first(...) __VA_OPT__(MA_ABORT("Junk before the first entry in a `REFL_MEMBERS` invocation."))

#define REFL_MEMBERS_impl_decl_low(...) MA_APPEND_TO_VA_END(_end, REFL_MEMBERS_impl_decl_loop_a __VA_ARGS__ )

#define REFL_MEMBERS_impl_decl_loop_a(...) REFL_MEMBERS_impl_decl_body(__VA_ARGS__) REFL_MEMBERS_impl_decl_loop_b
#define REFL_MEMBERS_impl_decl_loop_b(...) REFL_MEMBERS_impl_decl_body(__VA_ARGS__) REFL_MEMBERS_impl_decl_loop_a
#define REFL_MEMBERS_impl_decl_loop_a_end
#define REFL_MEMBERS_impl_decl_loop_b_end

#define REFL_MEMBERS_impl_decl_body(...) REFL_MEMBERS_impl_decl_body_low(__VA_ARGS__)
#define REFL_MEMBERS_impl_decl_body_low(params, ...) \
    MA_IF_NOT_EMPTY_ELSE(REFL_MEMBERS_impl_decl_body_low_members, REFL_MEMBERS_impl_decl_body_low_text, params)(params, __VA_ARGS__)

#define REFL_MEMBERS_impl_decl_body_low_text(params, ...) __VA_ARGS__

#define REFL_MEMBERS_impl_decl_body_low_members(params, ...) \
    ::Meta::identity_t<MA_PARAMS_FIRST(params)> /* type */\
    MA_NULL MA_VA_FOR_EACH( /* variables */\
        (MA_PARAMS_GET_ONE(, ReflMemberDecl, ReflInit, params, MA_PARAMS_IDENTITY)), /* initializer */\
        REFL_MEMBERS_impl_decl_member, \
        MA_TR_C(__VA_ARGS__) /* variable names */\
    ) () \
    ;

#define REFL_MEMBERS_impl_decl_member(init, index, name) (,) name MA_IDENTITY init MA_IDENTITY

// Internal. Generates common metadata for member variables.
#define REFL_MEMBERS_impl_metadata_generic(...) \
    MA_CALL(REFL_MEMBERS_impl_metadata_generic_low, REFL_MEMBERS_impl_skip_first __VA_ARGS__)

#define REFL_MEMBERS_impl_metadata_generic_low(...) \
    using member_ptrs = ::Meta::value_list< \
        /* can't use `MA_IDENTITY` here, since it would conflict with the same macro in `REFL_STRUCT_impl_low_base` */\
        MA_IDENTITY2( MA_NULL MA_APPEND_TO_VA_END(_end, REFL_MEMBERS_impl_metadata_memptr_loop_a __VA_ARGS__) () ) \
    >; \
    using member_attribs = ::Meta::type_list< \
        MA_APPEND_TO_VA_END(_end, REFL_MEMBERS_impl_metadata_memattr_loop_0 __VA_ARGS__) \
    >;

// Internal. Generates member pointers (as a part of common metadata).
#define REFL_MEMBERS_impl_metadata_memptr_loop_a(...) REFL_MEMBERS_impl_metadata_memptr_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memptr_loop_b
#define REFL_MEMBERS_impl_metadata_memptr_loop_b(...) REFL_MEMBERS_impl_metadata_memptr_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memptr_loop_a
#define REFL_MEMBERS_impl_metadata_memptr_loop_a_end
#define REFL_MEMBERS_impl_metadata_memptr_loop_b_end

#define REFL_MEMBERS_impl_metadata_memptr_body(params, ...) \
    MA_IF_NOT_EMPTY_ELSE(REFL_MEMBERS_impl_metadata_memptr_body_low, MA_NULL, params)(params, __VA_ARGS__)

#define REFL_MEMBERS_impl_metadata_memptr_body_low(params, ...) MA_VA_FOR_EACH(, REFL_MEMBERS_impl_metadata_memptr_pointer, MA_TR_C(__VA_ARGS__) )
#define REFL_MEMBERS_impl_metadata_memptr_pointer(data, index, name) (,) &t::name MA_IDENTITY

// Internal. Generates member attributes (as a part of common metadata).
#define REFL_MEMBERS_impl_metadata_memattr_loop_0(...)   REFL_MEMBERS_impl_metadata_memattr_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memattr_loop_a
#define REFL_MEMBERS_impl_metadata_memattr_loop_a(...) , REFL_MEMBERS_impl_metadata_memattr_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memattr_loop_b
#define REFL_MEMBERS_impl_metadata_memattr_loop_b(...) , REFL_MEMBERS_impl_metadata_memattr_body(__VA_ARGS__) REFL_MEMBERS_impl_metadata_memattr_loop_a
#define REFL_MEMBERS_impl_metadata_memattr_loop_0_end
#define REFL_MEMBERS_impl_metadata_memattr_loop_a_end
#define REFL_MEMBERS_impl_metadata_memattr_loop_b_end

#define REFL_MEMBERS_impl_metadata_memattr_body(params, ...) \
    MA_IF_NOT_EMPTY_ELSE(REFL_MEMBERS_impl_metadata_memattr_body_low0, MA_NULL, params)(MA_PARAMS_GET_ONE(, ReflMemberDecl, ReflAttr, params, MA_PARAMS_IDENTITY), __VA_ARGS__)

#define REFL_MEMBERS_impl_metadata_memattr_body_low0(maybe_attr, ...) \
    ::Refl::impl::Struct::Attr< \
        MA_VA_FOR_EACH(, REFL_MEMBERS_impl_metadata_memattr_plus1, MA_TR_C(__VA_ARGS__)) \
        MA_IF_NOT_EMPTY(MA_COMMA() maybe_attr, maybe_attr) \
    >

#define REFL_MEMBERS_impl_metadata_memattr_plus1(data, index, name) +1


// Internal. Generates metadata related to member names. (Not included in the common metadata).
#define REFL_MEMBERS_impl_metadata_memname(...) \
    MA_CALL(REFL_MEMBERS_impl_metadata_memname_low, REFL_MEMBERS_impl_skip_first __VA_ARGS__)

#define REFL_MEMBERS_impl_metadata_memname_low(...) \
    /* can't use a static array here, because static arrays are not allowed in classes declared inside of functions */\
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


REFL_STRUCT(MyStruct REFL_UNTRACKED_BASES std::vector<int> REFL_BASE std::string REFL_BASE std::string_view REFL_VIRTUAL_BASE std::vector<float>)
{

};

REFL_STRUCT(X REFL_PARAM auto,V REFL_PARAM typename,T,void REFL_POLYMORPHIC )
{

};

REFL_STRUCT(A)
{
    REFL_MEMBERS(
        REFL_DECL(int REFL_INIT =0) x, y, z
        REFL_DECL(float REFL_ATTR int) w, ww
        REFL_DECL(float REFL_ATTR int) h, hh
    )
};
