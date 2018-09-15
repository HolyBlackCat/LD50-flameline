#pragma once

#include <array>
#include <string>
#include <tuple>
#include <type_traits>

#include "interface.h"
#include "utils/macro.h"
#include "utils/meta.h"

#define ReflectStruct(class_name, seq) struct class_name { Reflect(class_name) seq };

#define Reflect(class_name) \
    /* Friending */\
    friend ::Refl::Custom::impl::Macro; \
    /* Alias for this type */\
    using _refl_this_type = class_name; \
    /* This type name */\
    static constexpr const char *_refl_name = #class_name; \
    /* Macro to pass field data to */\
    REFL_Structure

#define REFL_Structure(...) REFL_Structure_Seq(MA_VA_TO_SEQ_TRAILING_COMMA(__VA_ARGS__))
#define REFL_Structure_Seq(seq) \
    /* Field declarations */\
    MA_SEQ_FOR_EACH(REFL_Structure_FieldDeclPack, MA_NULL, , seq) \
    /* Tuple of member pointers (note that this has to be defined after the field declarations, otherwise we wouldn't be able to make the member pointers) */\
    static constexpr ::std::tuple _refl_member_pointers{ MA_SEQ_FOR_EACH(REFL_Structure_MemPointerPack, MA_NULL, , seq) };\
    /* Reflection interface */\
    struct _refl_interface \
    { \
        /* This type name */\
        inline static const ::std::string name = _refl_name; \
        /* Field count */\
        static constexpr int field_count = ::std::tuple_size_v<decltype(_refl_member_pointers)>; \
        /* Field access */\
        template <int I> static constexpr auto &field(_refl_this_type &ref) {return ref .* ::std::get<I>(_refl_member_pointers);} \
        /* Field names */\
        static std::string field_name(int index) {return ::std::array{ MA_SEQ_FOR_EACH(REFL_Structure_FieldNamePack, MA_NULL, , seq) }[index];} \
        /* Field categories */\
        static constexpr ::Refl::FieldCategory field_category(int index) {return ::std::array{ MA_SEQ_FOR_EACH(REFL_Structure_FieldCategoryPack, MA_NULL, , seq) }[index];} \
        /* Note that we don't define `is_tuple` here. It's defined later by the specialization of `Refl::Custom::Structure` for macro-reflected structures. */\
        /* It's set to 1 if `using _refl_structure_tuple_tag = void;` is detected in the reflected structure. */\
    }; \


// Field declarations
#define REFL_Structure_FieldDeclPack(i, unused, seq)            MA_OVERLOAD(REFL_Structure_FieldDeclPack_, MA_SEQ_TO_VA_PARENS(seq))
#define REFL_Structure_FieldDeclPack_1(x)                       MA_IDENTITY x
#define REFL_Structure_FieldDeclPack_2(      type, names      ) REFL_Structure_FieldDeclPack_3(type, names, ())
#define REFL_Structure_FieldDeclPack_3(      type, names, init) ::std::enable_if_t<1, MA_IDENTITY type> MA_VA_FOR_EACH_A(REFL_Structure_FieldDecl, MA_COMMA, init, MA_IDENTITY names) ;
#define REFL_Structure_FieldDeclPack_4(mode, type, names, init) REFL_Structure_FieldDeclPack_3(type, names, init)
#define REFL_Structure_FieldDecl(i, init, name)                 name MA_IDENTITY init

// Field member pointers
#define REFL_Structure_MemPointerPack(i, unused, seq)            MA_OVERLOAD(REFL_Structure_MemPointerPack_, MA_SEQ_TO_VA_PARENS(seq))
#define REFL_Structure_MemPointerPack_1(x)                       // Nothing.
#define REFL_Structure_MemPointerPack_2(      type, names      ) REFL_Structure_MemPointerPack_Low(names)
#define REFL_Structure_MemPointerPack_3(      type, names, init) REFL_Structure_MemPointerPack_Low(names)
#define REFL_Structure_MemPointerPack_4(mode, type, names, init) REFL_Structure_MemPointerPack_Low(names)
#define REFL_Structure_MemPointerPack_Low(names)                 MA_VA_FOR_EACH_A(REFL_Structure_MemPointer, MA_NULL, , MA_IDENTITY names)
#define REFL_Structure_MemPointer(i, unused, name)               &_refl_this_type::name,

// Field names
#define REFL_Structure_FieldNamePack(i, unused, seq)            MA_OVERLOAD(REFL_Structure_FieldNamePack_, MA_SEQ_TO_VA_PARENS(seq))
#define REFL_Structure_FieldNamePack_1(x)                       // Nothing.
#define REFL_Structure_FieldNamePack_2(      type, names      ) REFL_Structure_FieldNamePack_Low(names)
#define REFL_Structure_FieldNamePack_3(      type, names, init) REFL_Structure_FieldNamePack_Low(names)
#define REFL_Structure_FieldNamePack_4(mode, type, names, init) REFL_Structure_FieldNamePack_Low(names)
#define REFL_Structure_FieldNamePack_Low(names)                 MA_VA_FOR_EACH_A(REFL_Structure_FieldName, MA_NULL, , MA_IDENTITY names)
#define REFL_Structure_FieldName(i, unused, name)               #name,

// Field categories
#define REFL_Structure_FieldCategoryPack(i, unused, seq)            MA_OVERLOAD(REFL_Structure_FieldCategoryPack_, MA_SEQ_TO_VA_PARENS(seq))
#define REFL_Structure_FieldCategoryPack_1(x)                       // Nothing.
#define REFL_Structure_FieldCategoryPack_2(      type, names      ) REFL_Structure_FieldCategoryPack_Low(names, default_category)
#define REFL_Structure_FieldCategoryPack_3(      type, names, init) REFL_Structure_FieldCategoryPack_Low(names, default_category)
#define REFL_Structure_FieldCategoryPack_4(mode, type, names, init) REFL_Structure_FieldCategoryPack_Low(names, MA_IDENTITY mode)
#define REFL_Structure_FieldCategoryPack_Low(names, mode)           MA_VA_FOR_EACH_A(REFL_Structure_FieldCategory, MA_NULL, mode, MA_IDENTITY names)
#define REFL_Structure_FieldCategory(i, mode, name)                 (::Refl::FieldCategory::mode),


namespace Refl::Custom
{
    namespace impl
    {
        struct Macro // This is a class because we need to be able to friend it from within the macros.
        {
            template <typename T, typename = void> struct impl {};
            template <typename T> struct impl<T, std::void_t<typename T::_refl_interface>> {using interface = typename T::_refl_interface;};
        };

        template <typename T> using structure_tuple_tag = typename T::_refl_structure_tuple_tag;
    }

    template <typename T> struct Structure<T, std::void_t<typename impl::Macro::impl<T>::interface>> : impl::Macro::impl<T>::interface
    {
        static constexpr bool is_tuple = Meta::is_detected<impl::structure_tuple_tag, T>;
    };
}
