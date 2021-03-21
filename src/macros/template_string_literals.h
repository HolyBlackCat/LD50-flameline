#pragma once

namespace Macro
{
    template <char ...C>
    struct CexprStringHelper {};

    #ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
    #endif

    template <typename T, T ...C>
    [[nodiscard]] CexprStringHelper<C...> operator""_cexpr_string_literal()
    {
        return {};
    }

    #ifdef __clang__
    #pragma clang diagnostic pop
    #endif
}

using Macro::operator""_cexpr_string_literal;

// Usage: `my_function(LIT("foo"))`.
// Where `my_function` is declared as `template <char ...C> void my_function(LIT_PARAM(C))`.
#define LIT(str) LIT_impl(str)
#define LIT_impl(str) str##_cexpr_string_literal

#define LIT_PARAM(param) ::Macro::CexprStringHelper<param...>
