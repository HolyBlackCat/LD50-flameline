#pragma once

// This header includes libfmt headers, and adds some convenience macros.

/* TL;DR of the macros:
 *
 * The simple macro: `FMT("format", ...)`.
 *   Works like `fmt::format("format", ...)`, but checks the format string at compile-time (so it must be a string literal).
 *   Actually is a shorthand for `fmt::format(FMT_STRING("format"), ...)`.
 *
 * The fancy macro: `STR(...)`
 *   Concatenates arguments and builds a format string automatically under the hood.
 *   The arguments can either be string literals (which go into the format string)
 *   or parenthesized expressions (`"{...}"` is added to the format string for them).
 *   A parenthesized expression can be followed by a format specifier, which can either be a string literal,
 *   or, if you need variable width and/or precision, a parenthesized comma-separated list, where each element
 *   is either a string literal, or a parenthesized expression.
 *   Examples:
 *     STR("a = ", (a))                // Similar to `fmt::format("a = {}", a)`.
 *     STR("a = ", (a)"02x")           // Similar to `fmt::format("a = {:02x}", a)`.
 *     STR("a = ", (a)(".",(b),"g"))`. // Similar to `fmt::format("a = {:.{}g}", a, b))`
 *   In the string literals braces are escaped, so you can use them safely.
 *   Resulting format strings are validated at compile-time (not by `FMT_STRING` itself, but in a similar way).
 *
 * To use the custom syntax with other libfmt functions,
 *   use `FORMAT_ARGS_SIMPLE` (`FMT`-style) and `FORMAT_ARGS` (`STR`-style).
 *   Both expand to a comma-separated list of arguments (format string, followed by other arguments),
 *   that can be passed into various libfmt functions.
 *   `FORMAT_ARGS_SIMPLE` is mostly useless, because you might as well use `FMT_STRING` directly.
 *
 * Alternative prefixed names:
 *   If `FMT` and `STR` interfer with something, you can undefine them and use `FORMAT_FMT` and `FORMAT_STR` instead.
 *
 * Non-`char` strings support:
 *   `FMT` and related macros support non-char strings natively.
 *   `STR` and related macros have overloads with `_` suffix. They have an additional first parameter
 *   for the string literal prefix (e.g. `L`), which can be empty.
 */

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <string_view>
#include <string>
#include <type_traits>
#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "macros/generated.h"

// Import the UDLs.
// We only import `_a` and not `_format`.
// Instead of `_format`, use the `FMT()` macro defined below.
using fmt::literals::operator""_a;


namespace Strings
{
    namespace impl::Format
    {
        template <typename T>
        inline constexpr T default_format_string[] = {'{', '}', '\0'};
    }

    // A simple function that concatenates `params` and writes them to the `iterator`.
    // Uses libfmt, with the default format for all types.
    template <typename C = char, typename T, typename ...P>
    T ConcatTo(T iterator, const P &... params)
    {
        ((iterator = fmt::format_to(iterator, impl::Format::default_format_string<C>, params)), ...);
        return iterator;
    }

    // A simple function that concatenates `params` and returns the result as a string.
    // Uses libfmt, with the default format for all types.
    template <typename C = char, typename ...P>
    [[nodiscard]] std::basic_string<C> Concat(const P &... params)
    {
        std::basic_string<C> ret;
        ConcatTo<C>(std::back_inserter(ret), params...);
        return ret;
    }


    // Internal, used by the macros below.
    namespace impl::Format
    {
        // Constructs a format string by concatenating the strings from `list`.
        // Writes the string into `output` if it's not null, without a null terminator. Returns the string length.
        // Each string in the list must be prefixed either with `\f` or `\a`. The prefixes are removed during concatenation.
        // In `\a` strings, braces are escaped. `\f` strings are not modified.
        // `C` is a character type, normally `char`.
        template <typename C>
        constexpr std::size_t MakeString(std::initializer_list<std::basic_string_view<C>> list, C *output = nullptr)
        {
            std::size_t len = 0;
            for (std::basic_string_view<C> elem : list)
            {
                bool escape = elem[0] == '\a';

                for (std::size_t i = 1/*sic*/; i < elem.size(); i++)
                {
                    if (output)
                        *output++ = elem[i];
                    len++;

                    if (escape && (elem[i] == '{' || elem[i] == '}'))
                    {
                        if (output)
                            *output++ = elem[i];
                        len++;
                    }
                }
            }
            return len;
        }
    }
}


// -- THE SIMPLE MACROS --

// A wrapper/replacement for `fmt::format(...)`, accepts parameters in the same way.
// Automatically wraps the first argument in `FMT_STRING(...)`, enabling compile-time format string validation.
// The first argument must be a string literal.
// Uses `FORMAT_ARGS_SIMPLE` under the hood.
#define FMT(...) FORMAT_FMT(__VA_ARGS__)
#define FORMAT_FMT(...) ::fmt::format(FORMAT_ARGS_SIMPLE(__VA_ARGS__))

// A convenience macro. `FORMAT_ARGS_SIMPLE(string, ...)` expands to `FMT_STRING(string), ...`,
// where `FMT_STRING` is a libfmt macro that enables the compile-time format string validation.
// Primarily useful for writing other macros.
// It's callable even with a single parameter. There's no max limit on the amount of parameters.
#define FORMAT_ARGS_SIMPLE(...) FMT_STRING(MA_VA_FIRST(__VA_ARGS__)) MA_IF_COMMA_ELSE(FORMAT_impl_args, MA_NULL, __VA_ARGS__)(__VA_ARGS__)
#define FORMAT_impl_args(format, ...) , __VA_ARGS__


// -- THE FANCY MACROS --

// A macro that concatenates its arguments using `fmt::format`.
// String literals can be passed as is, and everything else must be wrapped in parentheses.
// A parenthesis can be followed by a string literal, if custom formatting is necessary.
// Example: `STR("a = ", (a), ", b = ", (b)"02x")`. This expands roughly to `fmt::format(FMT_STRING("a = {}, b = {:02x}"), a, b)`.
// If you need to specify variable width and/or precision for a field, then the format specifier itself must be wrapped in parentheses,
// and be a comma separated list of string literals and parenthesized expressions.
// Example: `STR("a = ", (a)(".",(b),"g"))`. This expands roughly to `fmt::format(FMT_STRING("a = {:.{}g}", a, b))`
// To use other libfmt functions with this syntax, use `FORMAT_ARGS(...)` macro.
// The amount of allowed parameters is limited by looping capabilities of `macros/generated.h`. At the time of writing this, you can get around 64 parameters.
#define STR(...) FORMAT_STR(__VA_ARGS__)
// Similar to `STR(...)`, but additionally lets you specify the string literal prefix, such as `L`. The prefix can be empty.
#define STR_(prefix, ...) FORMAT_STR_(prefix, __VA_ARGS__)
// Another name for `STR(...)`.
#define FORMAT_STR(...) ::fmt::format(FORMAT_ARGS(__VA_ARGS__))
// Another name for `STR_(...)`.
#define FORMAT_STR_(prefix, ...) ::fmt::format(FORMAT_ARGS_(prefix, __VA_ARGS__))

// A convenience macro. Expands to a compile-time format string (similar to `FMT_STRING`), followed by a comma-separate argument list.
// See the comments on `STR(...)` for the syntax.
#define FORMAT_ARGS(...) FORMAT_ARGS_(, __VA_ARGS__)
// Similar to `FORMAT_ARGS`, but additionally lets you specify the string literal prefix, such as `L`. The prefix can be empty.
#define FORMAT_ARGS_(prefix, ...) FORMAT_impl_inline_str( MA_VA_FOR_EACH(prefix, FORMAT_impl_inline_format, MA_TR_C(__VA_ARGS__)) ) MA_VA_FOR_EACH(, FORMAT_impl_inline_param, MA_TR_C(__VA_ARGS__))

// Internal. Loop body for creating a format string. `elem` is either a string literal or a field (a parenthesized expression, possibly followed by a format specification); see comments on `STR(...)` for details.
// Dispatches between `FORMAT_impl_inline_format_field` and `FORMAT_impl_inline_format_str` depending on whether the argument starts with `(...)` or not.
#define FORMAT_impl_inline_format(prefix, i, elem) MA_IF_COMMA_ELSE(FORMAT_impl_inline_format_field, FORMAT_impl_inline_format_str, MA_COMMA elem)(prefix, i, elem)

// Internal. Expands to `"\a"`, followed by `elem`, which is supposed to be a string literal.
#define FORMAT_impl_inline_format_str(prefix, i, elem) MA_CAT(prefix, "\a") elem,

// Internal. Strips leading `(...)` from `elem` and calls `FORMAT_impl_inline_format_field_low`.
#define FORMAT_impl_inline_format_field(prefix, i, elem) FORMAT_impl_inline_format_field_low(prefix, i, MA_NULL elem)
// Internal. If `...` is empty, expands to `"\f{}"`. Otherwise expands to `"\f{:" ... "}"`,
// where `...` is the expansion of either `FORMAT_impl_inline_format_field_list` or `FORMAT_impl_inline_format_field_simple`,
// depending on whether `elem` starts with a `(...)` or not.
#define FORMAT_impl_inline_format_field_low(prefix, i, ...) MA_CAT(prefix, "\f{") \
    __VA_OPT__( ":" MA_IF_COMMA_ELSE(FORMAT_impl_inline_format_field_list, FORMAT_impl_inline_format_field_simple, MA_COMMA __VA_ARGS__)(i, __VA_ARGS__) ) "}",
// Internal. Expands to `elem`, which is supposed to be a format specified in quotes.
#define FORMAT_impl_inline_format_field_simple(i, elem) elem
// Internal. `elem` is a parenthesized comma-separated list of strings and parenthesized expressions; see comments on `STR(...)` for details.
// Invokes `FORMAT_impl_inline_format_field_list_elem` on each element.
#define FORMAT_impl_inline_format_field_list(i, elem) MA_VA_FOR_EACH_1(i,, FORMAT_impl_inline_format_field_list_elem, MA_TR_C elem)
// Internal. Loop body. If `elem` is parenthesized, expands to `"{}"`, otherwise expands to `elem` (which is then supposed to be a string literal).
#define FORMAT_impl_inline_format_field_list_elem(d, i, elem) MA_IF_COMMA_ELSE("{}", elem, MA_COMMA elem)

// Internal. Loop body for creating a list of parameters for some format string.
// If `elem` is parenthesized, calls `FORMAT_impl_inline_param_low`, otherwise does nothing.
#define FORMAT_impl_inline_param(d, i, elem) MA_IF_COMMA_ELSE(FORMAT_impl_inline_param_low, MA_NULL, MA_COMMA elem)(i, elem)
// Internal. Expands to `, elem` (`elem` is an expression), followed by expansion of `FORMAT_impl_inline_param_low_a`,
// which contains parameters for nested fields like width and precision, and could be empty.
#define FORMAT_impl_inline_param_low(i, elem) , MA_SEQ_FIRST(elem) FORMAT_impl_inline_param_low_a(i, MA_SEQ_NO_FIRST(elem))
// Internal. `elem` is a format specifier, possibly parenthesized.
// If it's parenthesized, passes it to `FORMAT_impl_inline_param_list`, otherwise does nothing.
#define FORMAT_impl_inline_param_low_a(i, elem) MA_IF_COMMA_ELSE(FORMAT_impl_inline_param_list, MA_NULL, MA_COMMA elem)(i, elem)
// Internal. `elem` is a parenthesized comma-separated list of strings and parenthesized expressions; see comments on `STR(...)` for details.
// Invokes `FORMAT_impl_inline_param_list_elem` on each element.
#define FORMAT_impl_inline_param_list(i, elem) MA_VA_FOR_EACH_1(i,, FORMAT_impl_inline_param_list_elem, MA_TR_C elem)
// Internal. Loop body. If argument is parenthesized, calls `FORMAT_impl_inline_param_list_elem_low`, otherwise does nothing.
#define FORMAT_impl_inline_param_list_elem(d, i, elem) MA_IF_COMMA_ELSE(FORMAT_impl_inline_param_list_elem_low, MA_NULL, MA_COMMA elem)(elem)
// Internal. Removes `(...)` surrounding `elem`, and adds a comma before it.
#define FORMAT_impl_inline_param_list_elem_low(elem) , MA_IDENTITY elem

// Internal. Constructs a format string object from pieces.
// `...` is a comma-separated list of string literals, see `Strings::impl::Format::MakeString` for their meaning.
// Returns a class instance that can be used as a format string for libfmt functions.
#define FORMAT_impl_inline_str(...) \
    []{ \
        using C = ::std::remove_cvref_t<decltype(*MA_VA_FIRST(__VA_ARGS__))>; \
        static constexpr auto string = []{ \
            ::std::array<C, ::Strings::impl::Format::MakeString<C>({__VA_ARGS__})> string; \
            ::Strings::impl::Format::MakeString<C>({__VA_ARGS__}, string.data()); \
            return string; \
        }(); \
        /* The below part mimics `FMT_STRING`. */\
        struct S : ::fmt::compile_string \
        { \
            /* Not sure if this alias is necessary, but `FMT_STRING` has one, so I'll keep it for a good measure. */\
            using char_type = C; \
            [[maybe_unused]] constexpr operator ::fmt::basic_string_view<char_type>() const \
            { \
                return ::fmt::basic_string_view<char_type>(string.data(), string.size()); \
            } \
        }; \
        return S{}; \
    }()
