#pragma once

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>

#include <double-conversion/double-conversion.h>

#include "macros/check.h"
#include "meta/misc.h"
#include "meta/type_info.h"
#include "program/errors.h"
#include "strings/escape.h"
#include "utils/robust_math.h"

namespace Strings
{
    namespace impl
    {
        // Uses the most suitable `std::strto*` for the specified type.
        // The return type might be wider than T.
        template <
            typename T,
            CHECK((std::is_integral_v<T> && sizeof(T) <= sizeof(long long)) || (std::is_floating_point_v<T> && sizeof(T) <= sizeof(long double)))
        >
        auto strto_low(const char *str, char **str_end, int base = 0)
        {
            if constexpr (std::is_integral_v<T>)
            {
                constexpr bool use_long = sizeof(T) <= sizeof(long);
                constexpr bool is_signed = std::is_signed_v<T>;

                if constexpr (use_long && is_signed)
                    return std::strtol(str, str_end, base);
                else if constexpr (use_long && !is_signed)
                    return std::strtoul(str, str_end, base);
                else if constexpr (!use_long && is_signed)
                    return std::strtoll(str, str_end, base);
                else // !use_long && !is_signed
                    return std::strtoull(str, str_end, base);
            }
            else
            {
                if constexpr (sizeof(T) <= sizeof(float))
                    return std::strtof(str, str_end);
                else if constexpr (sizeof(T) <= sizeof(double))
                    return std::strtod(str, str_end);
                else if constexpr (sizeof(T) <= sizeof(long double))
                    return std::strtold(str, str_end);
            }
        }

        template <typename T>
        [[noreturn]] void ConversionFailure(std::string_view str, std::string_view message = "")
        {
            Program::Error("Unable to convert `", Escape(str), "` to ", Meta::TypeName<T>(), message.empty() ? "." : std::string(": ").append(message).append("."));
        }

        inline constexpr const char *string_inf = "inf", *string_nan = "nan";
        inline constexpr char
            char_digit_sep = '\'', // Optional character separator.
            char_exp = 'e', // Exponent sign.
            char_long_double_parts_sep = '@'; // Separates two parts of a `long double`, see `ToString()` for details. Don't set this to a symbol that can normally occur in numbers.

        inline constexpr auto conv_str_to_real_flags =
            double_conversion::StringToDoubleConverter::ALLOW_HEX |
            double_conversion::StringToDoubleConverter::ALLOW_CASE_INSENSITIVITY |
            double_conversion::StringToDoubleConverter::ALLOW_HEX_FLOATS;
        inline const double_conversion::StringToDoubleConverter conv_str_to_real(conv_str_to_real_flags, 0, std::numeric_limits<double>::signaling_NaN(), string_inf, string_nan, char_digit_sep);

        // The numeric parameters passed to the constructor come from the `DoubleToStringConverter::EcmaScriptConverter()`, aka the default converter settings.
        // They should be sensible enough.
        inline const double_conversion::DoubleToStringConverter conv_real_to_str(conv_real_to_str.EMIT_POSITIVE_EXPONENT_SIGN, string_inf, string_nan, char_exp, -6, 21, 6, 0);
    }

    // Converts a string to a number using one of the standard `strto*` functions.
    // Use `FromString` instead if possible, as it has more features.
    // `*str_end` is set to the next byte after the end of the number. On failure it's set to `str`.
    // `base` only works for integral types, and can be between 2 and 36. If it's 0, the base is detected automatically.
    // Skips leading spaces. Ignores any symbols after the number.
    // Shouldn't throw.
    // If `T` is unsigned, negative values appear to be allowed (if they are greater than -2^n). Otherwise out-of-range values are not allowed.
    template <typename T, CHECK(std::is_arithmetic_v<T>)>
    [[nodiscard]] T strto(const char *str, const char **str_end, int base = 0)
    {
        char *end = const_cast<char *>(str);
        errno = 0; // `strto*` appears to indicate out-of-range errors only by setting `errno`.
        auto raw_result = impl::strto_low<T>(str, &end, base);
        if (end == str || errno != 0)
        {
            *str_end = str;
            return 0;
        }

        T result = raw_result;
        if (Robust::not_equal(result, raw_result))
        {
            *str_end = str;
            return 0;
        }

        *str_end = end;
        return result;
    }

    // Returns the max buffer size that `ToString()` needs to successfully do its job.
    inline constexpr std::size_t ToStringMaxBufferLen()
    {
        // Length of the string "false".
        constexpr std::size_t b = 5;
        // `digits10` is the amount of digits that can represented exactly, so we add one extra digit. We also might need space for a sign, so we add one more.
        constexpr std::size_t i = std::numeric_limits<unsigned long long>::digits10 + 2;
        // Since we represent long doubles as pairs of regular doubles, we multiply by 2. We also add 1 for a number separator.
        constexpr std::size_t f = double_conversion::DoubleToStringConverter::kBase10MaximalLength * 2 + 1;
        // `+1` is for the null terminator.
        return std::max({b, i, f}) + 1;
    }

    // Returns `'`, the digit separator.
    inline constexpr char CharDigitSeparator() {return impl::char_digit_sep;}
    // Returns `@`, which separates parts of a long double.
    inline constexpr char CharLongDoublePartsSeparator() {return impl::char_long_double_parts_sep;}

    // Converts a number to a string.
    // Returns `false` on failure, which shouldn't happen if the buffer is at least `ToStringMaxBufferLen()` large.
    // If `T` is floating-point, the shortest unique representation is used.
    // If `T` is a `long double`, it's converted to a pair of `@` separated numbers, that need to be
    //   added together to get the original number. The first number is always as large as possible.
    // If the number is NaN, outputs `nan`. If it's an infinity, outputs `inf` or `-inf` depending on the sign.
    template <
        Meta::deduce...,
        typename T,
        CHECK((std::is_integral_v<T> && sizeof(T) <= sizeof(long long)) || (std::is_floating_point_v<T> && sizeof(T) <= sizeof(long double)))
    >
    bool ToString(char *buffer, std::size_t buffer_size, T number)
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            if (buffer_size < (number ? 5/*true\0*/ : 6/*false\0*/))
                return false;
            std::strcpy(buffer, number ? "true" : "false");
        }
        else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
        {
            if (std::snprintf(buffer, buffer_size, "%lld", (long long)number) <= 0)
                return false;
        }
        else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
        {
            if (std::snprintf(buffer, buffer_size, "%llu", (unsigned long long)number) <= 0)
                return false;
        }
        else if constexpr (sizeof(T) <= sizeof(double))
        {
            // Check for an empty buffer early, because as soon as `str` is constructed with an empty buffer,
            // it appears to be toast (the destructor will either fail with an assertion, or access the buffer out of bounds).
            if (buffer_size == 0)
                return false;

            double_conversion::StringBuilder str(buffer, buffer_size);

            bool ok;
            if constexpr (sizeof(T) <= sizeof(float))
                ok = impl::conv_real_to_str.ToShortestSingle(number, &str);
            else
                ok = impl::conv_real_to_str.ToShortest(number, &str);

            if (!ok || str.position() >= str.size())
            {
                // Either the conversion failed, or there is no space for the null-terminator.
                // The destructor of `str` will try to null-terminate the string. We don't want it to write
                // the null-terminator out of bounds, so we reset the string builder.
                str.Reset();
                return false;
            }
        }
        else if constexpr (sizeof(T) <= sizeof(long double))
        {
            // Check if any long double can be represented exactly as a sum of two regular doubles.
            static_assert(std::numeric_limits<double>::digits * 2 >= std::numeric_limits<long double>::digits, "Your long doubles are too large!");

            if (buffer_size == 0) // See above for explanation.
                return false;

            double major = number;
            double minor = number - major;

            double_conversion::StringBuilder str(buffer, buffer_size);
            if (!impl::conv_real_to_str.ToShortest(major, &str))
            {
                str.Reset(); // See above for explanation.
                return false;
            }

            // `isnan` prevents the second part from being generated if the number is an infinity with any sign or nan.
            if (minor == 0 || std::isnan(minor))
            {
                if (str.position() >= str.size())
                {
                    str.Reset();
                    return false;
                }
            }
            else
            {
                DebugAssert("ToString attempted to represent a long double as a pair of regular doubles, but the number didn't survive a round-trip conversion.", (long double)major + minor == number);

                if (str.position() + 1 >= str.size()) // We use +1 because we want space for both the delimiter and the null-terminator.
                {
                    str.Reset();
                    return false;
                }
                str.AddCharacter(impl::char_long_double_parts_sep);
                if (!impl::conv_real_to_str.ToShortest(minor, &str) || str.position() >= str.size())
                {
                    str.Reset();
                    return false;
                }
            }
        }
        else
        {
            static_assert(Meta::value<false, T>, "This type is not supported.");
        }

        return true;
    }

    template <typename T, CHECK_EXPR(ToString(nullptr, 0, T{}))>
    [[nodiscard]] std::string ToString(T number)
    {
        char buf[ToStringMaxBufferLen()];
        [[maybe_unused]] bool ok = ToString(buf, sizeof buf, number);
        DebugAssert("The internal buffer allocated by `ToString` ended up being too small, huh.", ok);
        return buf;
    }

    // Converts a string to a number. Throws on failure.
    // Any trailing and leading whitespace is ignored.
    // Should be able to handle any string generated by `ToString()`.
    // Can handle hex integers and floats, digit separators ('), and special float values ("inf", "+inf", "-inf", "nan").
    // If `T` is a `long double`, expects either a double, or two doubles separated with `@`, that will be
    //   added together to produce the result. This matches the output format of `ToString()`.
    template <
        typename T,
        CHECK((std::is_integral_v<T> && sizeof(T) <= sizeof(long long)) || (std::is_floating_point_v<T> && sizeof(T) <= sizeof(long double)))
    >
    [[nodiscard]] T FromString(std::string_view str)
    {
        static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>);

        // Strip leading and trailing whitespace.
        str = Trim(str);

        if constexpr (std::is_same_v<T, bool>)
        {
            if (str == "false")
                return false;
            if (str == "true")
                return true;
            // Otherwise fall back to integral conversion.
        }

        if constexpr (std::is_integral_v<T>)
        {
            // Copy the string to a temporary buffer to strip any character separators.
            char buf[ToStringMaxBufferLen()];
            std::size_t buf_pos = 0;

            bool prev_is_digit = 0;
            bool prev_is_separator = 0;
            for (char ch : str)
            {
                if (buf_pos >= sizeof buf - 1) // `-1` because we need space for a null-terminator.
                    impl::ConversionFailure<T>(str, "too long");

                bool is_digit = std::isdigit((unsigned char)ch);
                bool is_separator = ch == impl::char_digit_sep;

                // Make sure a separator is always preceeded by a digit.
                if (is_separator && !prev_is_digit)
                    impl::ConversionFailure<T>(str, "incorrect separator usage");

                if (!is_separator)
                    buf[buf_pos++] = ch;

                prev_is_digit = is_digit;
                prev_is_separator = is_separator;
            }

            // Stop if the number ends with a separator.
            if (prev_is_separator)
                impl::ConversionFailure<T>(str, "incorrect separator usage");

            buf[buf_pos] = '\0';

            const char *end = buf;
            T result = strto<T>(buf, &end);
            if (end != buf + buf_pos)
                impl::ConversionFailure<T>(str);

            return result;
        }
        else if constexpr (sizeof(T) <= sizeof(double))
        {
            // We have to explicitly check for an empty string.
            if (str.size() == 0)
                impl::ConversionFailure<T>(str);

            int chars_consumed = 0;
            T result;
            if constexpr (sizeof(T) <= sizeof(float))
                result = impl::conv_str_to_real.StringToFloat(str.data(), str.size(), &chars_consumed);
            else
                result = impl::conv_str_to_real.StringToDouble(str.data(), str.size(), &chars_consumed);

            // We can safely use this to check if the conversion succeeded, because the converter is configured
            // to disallow any extra whitespace and junk at the edges of the string.
            if (std::size_t(chars_consumed) != str.size())
                impl::ConversionFailure<T>(str);
            return result;
        }
        else // sizeof(T) <= sizeof(long double)
        {
            auto separator_pos = str.find_first_of(impl::char_long_double_parts_sep);
            if (separator_pos == str.npos)
                return FromString<double>(str);

            // Stop if the separator is at the edge of the string, or if it has whitespace around it.
            bool bad_separator = separator_pos == 0 || separator_pos == str.size() - 1 ||
                std::isspace((unsigned char)str[separator_pos-1]) || std::isspace((unsigned char)str[separator_pos+1]);
            if (bad_separator)
                impl::ConversionFailure<T>(str, Str("incorrect usage of ", impl::char_long_double_parts_sep));

            return (long double)FromString<double>(str.substr(0, separator_pos)) + FromString<double>(str.substr(separator_pos+1));
        }
    }

    // Works as either `ToString` (T = std::string) or `FromString()` (T is arithmetic).
    // Throws on failure.
    template <typename T, Meta::deduce..., typename U, CHECK(std::is_same_v<T, std::string>)>
    [[nodiscard]] std::string lexical_cast(U number)
    {
        return ToString(number);
    }
    template <typename T, Meta::deduce..., CHECK(std::is_arithmetic_v<T>)>
    [[nodiscard]] T lexical_cast(std::string_view string)
    {
        return FromString<T>(string);
    }
}
