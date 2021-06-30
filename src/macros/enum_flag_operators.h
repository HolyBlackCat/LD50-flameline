#pragma once

#include <type_traits>

#include "program/compiler.h"

// Synthesizes operators for a enum of flags: `&`, `|`, and `~`.
#define IMP_ENUM_FLAG_OPERATORS(name_) IMP_ENUM_FLAG_OPERATORS_CUSTOM([[nodiscard]] IMP_ALWAYS_INLINE static, name_)

// Same, but works at class scope.
#define IMP_ENUM_FLAG_OPERATORS_IN_CLASS(name_) IMP_ENUM_FLAG_OPERATORS_CUSTOM([[nodiscard]] IMP_ALWAYS_INLINE friend, name_)

// Same, but lets you specify a custom decl-specifier-seq.
#define IMP_ENUM_FLAG_OPERATORS_CUSTOM(decl_spec_seq_, name_) \
    [[maybe_unused]] decl_spec_seq_ name_ operator&(name_ a, name_ b) {return name_(::std::underlying_type_t<name_>(a) & ::std::underlying_type_t<name_>(b));} \
    [[maybe_unused]] decl_spec_seq_ name_ operator|(name_ a, name_ b) {return name_(::std::underlying_type_t<name_>(a) | ::std::underlying_type_t<name_>(b));} \
    [[maybe_unused]] decl_spec_seq_ name_ operator~(name_ a) {return name_(~::std::underlying_type_t<name_>(a));}
