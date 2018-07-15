#ifndef REFLECTION_H_INCLUDED
#define REFLECTION_H_INCLUDED

#include <array>
#include <charconv>
#include <cstddef>
#include <cstring>
#include <exception>
#include <limits>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>

namespace Refl
{
    namespace impl
    {
        constexpr bool is_alphanum(char ch)
        {
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_';
        }

        void skip_whitespace(const char *&string) // Also skips comments, from '#' to EOL.
        {
            while (1)
            {
                while (*string > '\0' && *string <= ' ')
                    string++;

                if (*string != '#')
                    return;

                string++;
                while (*string && *string != '\n')
                    string++;
            }
        }

        inline namespace Macro
        {
            constexpr int count_commas(const char *string)
            {
                int ret = 0;
                while (*string)
                {
                    if (*string == ',')
                        ret++;
                    string++;
                }
                return ret;
            }

            template <int N> constexpr std::array<char,N> zero_out_delimiters(const char (&str)[N])
            {
                std::array<char,N> ret{};
                for (int i = 0; i < N; i++)
                    ret[i] = (is_alphanum(str[i]) ? str[i] : '\0');
                return ret;
            }

            template <int N> constexpr std::array<const char *, N> extract_field_names(const char *string)
            {
                std::array<const char *, N> ret{};
                const char *cur = string;
                int index = 0;
                while (index != N)
                {
                    while (!is_alphanum(*cur++)) {}
                    ret[index] = cur-1;
                    while (is_alphanum(*cur++)) {}
                    index++;
                }
                return ret;
            }
        }

        template <typename T, typename = void> struct macro_refl_has_primary_fields_impl : std::false_type {};
        template <typename T> struct macro_refl_has_primary_fields_impl<T, std::void_t<decltype(T::_refl_field_count_primary)>> : std::true_type {};
        template <typename T> inline constexpr bool macro_refl_has_primary_fields_v = macro_refl_has_primary_fields_impl<T>::value;

        template <typename T, typename = void> struct macro_refl_has_optional_fields_impl : std::false_type {};
        template <typename T> struct macro_refl_has_optional_fields_impl<T, std::void_t<decltype(T::_refl_field_count_optional)>> : std::true_type {};
        template <typename T> inline constexpr bool macro_refl_has_optional_fields_v = macro_refl_has_optional_fields_impl<T>::value;

        template <typename T> inline constexpr bool is_macro_reflected_v = macro_refl_has_primary_fields_v<T> || macro_refl_has_optional_fields_v<T>;


        template <typename T, typename = void> struct is_specialized_impl : std::true_type {};
        template <typename T> struct is_specialized_impl<T, std::void_t<typename T::not_specialized_tag>> : std::false_type {};
        template <typename T> inline constexpr bool is_specialized_v = is_specialized_impl<T>::value;
    }

    inline namespace Utility
    {
        template <int ...I, typename F> void cexpr_foreach(std::integer_sequence<int, I...>, F &&func)
        {
            (func(std::integral_constant<int, I>{}) , ...);
        }

        template <int N, typename F> void cexpr_for(F &&func)
        {
            Refl::Utility::cexpr_foreach(std::make_integer_sequence<int, N>{}, std::forward<F>(func));
        }
    }

    namespace Custom // Customization points
    {
        // Declarations

        // Structures
        template <typename T, typename = void> struct Structure // T is never cv-qualified.
        {
            static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "Internal error: T must not be cv-qualified at this point.");
            using not_specialized_tag = void;

            static constexpr int field_count_primary()
            {
                return 0;
            }

            static constexpr int field_count_optional()
            {
                return 0;
            }

            // Field indices below are in [ 0 ; field_count_primary + field_count_optional ).
            // Out of them, the first `field_count_primary ` indices represent primary fields.

            static constexpr const char *field_name(int index) // `index` will never be out of range
            {
                (void)index;
                return ""; // A stub.
            }

            template <int I> static constexpr auto &field_lvalue_ref(T &object) // `I` will never be out of range
            {
                return object; // A stub.
            }
        };

        // Primitives
        template <typename T, typename = void> struct Primitive // T is never cv-qualified.
        {
            static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "Internal error: T must not be cv-qualified at this point.");
            using not_specialized_tag = void;

            static std::string to_string(const T &object)
            {
                (void)object;
                return "";
            }

            // On success, should return 1 and change `str` to the next unused byte.
            // On failure, should return 0. In this case, the pointer and `object` have undeterminate values.
            static bool from_string(T &object, const char *&str)
            {
                (void)str;
                (void)object;
                return 0;
            }
        };


        // Builtin implementations

        // Macro-reflected structures
        template <typename T> struct Structure<T, std::enable_if_t<impl::is_macro_reflected_v<T>>>
        {
            static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "Internal error: T must not be cv-qualified at this point.");

            static constexpr int field_count_primary()
            {
                if constexpr (impl::macro_refl_has_primary_fields_v<T>)
                    return T::_refl_field_count_primary;
                else
                    return 0;
            }

            static constexpr int field_count_optional()
            {
                if constexpr (impl::macro_refl_has_optional_fields_v<T>)
                    return T::_refl_field_count_optional;
                else
                    return 0;
            }

            static constexpr const char *field_name(int index)
            {
                if (index < field_count_primary())
                {
                    if constexpr (impl::macro_refl_has_primary_fields_v<T>)
                        return T::_refl_field_names_primary[index];
                    else
                        return "";
                }
                else
                {
                    if constexpr (impl::macro_refl_has_optional_fields_v<T>)
                        return T::_refl_field_names_optional[index - field_count_primary()];
                    else
                        return "";
                }
            }

            template <int I> static constexpr auto &field_lvalue_ref(T &object)
            {
                if constexpr (I < field_count_primary())
                    return std::get<I>(object._refl_fields_primary());
                else if constexpr (I - field_count_primary() < field_count_optional())
                    return std::get<I - field_count_primary()>(object._refl_fields_optional());
                else
                    return object; // A default value.
            }
        };

        // Arithmetic types
        template <typename T> struct Primitive<T, std::enable_if_t<std::is_arithmetic_v<T>>> // T is never cv-qualified.
        {
            static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "Internal error: T must not be cv-qualified at this point.");

            static std::string to_string(const T &object)
            {
                if constexpr (std::is_integral_v<T>)
                {
                    return std::to_string(object);
                }
                else
                {
                    constexpr const char *format = std::is_same_v<T, float>  ? "%.*g"  :
                                                   std::is_same_v<T, double> ? "%.*lg" :
                                                               /*long double*/ "%.*Lg";
                    constexpr int len = std::numeric_limits<T>::max_digits10;
                    char buffer[len*2];
                    std::snprintf(buffer, sizeof buffer, format, len, object);
                    return buffer;
                }
            }

            // On success, should return 1 and change `str` to the next unused byte.
            // On failure, should return 0. In this case, the pointer and `object` have undeterminate values.
            static bool from_string(T &object, const char *&str)
            {
                char *end;

                if constexpr (std::is_integral_v<T>)
                {
                    using result_t = std::conditional_t<sizeof(T) < sizeof(long), std::conditional_t<std::is_signed_v<T>, long, unsigned long>, T>;
                    result_t result;

                    constexpr bool is_long = sizeof(T) <= sizeof(long);
                    constexpr bool is_signed = std::is_signed_v<T>;

                         if constexpr ( is_long &&  is_signed) result = std::strtol(str, &end, 0);
                    else if constexpr ( is_long && !is_signed) result = std::strtoul(str, &end, 0);
                    else if constexpr (!is_long &&  is_signed) result = std::strtoll(str, &end, 0);
                    else            /* !is_long && !is_signed*/result = std::strtoull(str, &end, 0);

                    if (end == str)
                        return 0;

                    if constexpr (sizeof(T) < sizeof(result_t))
                        if (result < std::numeric_limits<T>::min() || result > std::numeric_limits<T>::max())
                            return 0;

                    object = result;
                    str = end;
                    return 1;
                }
                else
                {
                         if constexpr (std::is_same_v<T, float      >) object = std::strtof(str, &end);
                    else if constexpr (std::is_same_v<T, double     >) object = std::strtod(str, &end);
                    else            /* std::is_same_v<T, long double>*/object = std::strtold(str, &end);

                    if (end == str)
                        return 0;

                    str = end;
                    return 1;
                }
            }
        };

        // Bool
        template <> struct Primitive<bool, void>
        {
            static std::string to_string(const bool &object)
            {
                return object ? "true" : "false";
            }

            // On success, should return 1 and change `str` to the next unused byte.
            // On failure, should return 0. In this case, the pointer and `object` have undeterminate values.
            static bool from_string(bool &object, const char *&str)
            {
                if (!std::strncmp(str, "false", 5))
                {
                    str += 5;
                    object = 0;
                    return 1;
                }
                if (!std::strncmp(str, "false", 4))
                {
                    str += 4;
                    object = 1;
                    return 1;
                }
                return 0;
            }
        };
    }

    // Raw interfaces.
    // Any cv-qualifiers and ref-qualifiers on T are ignored.
    template <typename T> using StructureInterface = Custom::Structure<std::remove_cv_t<std::remove_reference_t<T>>>;
    template <typename T> using PrimitiveInterface = Custom::Primitive<std::remove_cv_t<std::remove_reference_t<T>>>;


    // Type classification:
    template <typename T> inline constexpr bool is_structure = impl::is_specialized_v<StructureInterface<T>>;
    template <typename T> inline constexpr bool is_primitive = impl::is_specialized_v<PrimitiveInterface<T>> && !is_structure<T>;
    template <typename T> inline constexpr bool is_not_reflected = !is_structure<T> && !is_primitive<T>;


    // Structures:
    template <typename T> inline constexpr int field_count_primary = StructureInterface<T>::field_count_primary();
    template <typename T> inline constexpr int field_count_optional = StructureInterface<T>::field_count_optional();
    template <typename T> inline constexpr int field_count = field_count_primary<T> + field_count_optional<T>;

    template <typename T> std::string field_name(int index) // Primary fields go first, then optional fields.
    {
        if (index >= 0 && index < field_count<T>)
            return StructureInterface<T>::field_name(index);
        else
            return std::to_string(index);
    }

    template <int I, typename T> decltype(auto) field(T &&object) // Primary fields go first, then optional fields.
    {
        using param_base_t = std::remove_cv_t<std::remove_reference_t<T>>;
        static_assert(is_structure<param_base_t>, "The type is not a reflected structure.");
        static_assert(I < field_count<param_base_t> && I >= 0, "Field index is out of range.");
        constexpr bool is_mutable = !std::is_const_v<std::remove_reference_t<T>>;
        constexpr bool is_lvalue = std::is_reference_v<T>;
        using result_base_t = std::remove_reference_t<decltype(StructureInterface<param_base_t>::template field_lvalue_ref<I>(std::declval<param_base_t &>()))>; // This type may be already cv-qualified.
        using result_cv_t = std::conditional_t<is_mutable, result_base_t, const result_base_t>;
        using result_t = std::conditional_t<is_lvalue, result_cv_t &, result_cv_t &&>;
        return static_cast<result_t>(StructureInterface<param_base_t>::template field_lvalue_ref<I>(const_cast<param_base_t &>(object)));
    }

    // If the template parameter is const, the type will be made const too.
    template <typename T, int I> using field_type = std::remove_reference_t<decltype(field<I>(std::declval<T &>()))>;

    template <typename T> int field_index(const std::string &name)
    {
        using type_without_cvref = std::remove_cv_t<std::remove_reference_t<T>>;
        if constexpr (std::is_same_v<type_without_cvref, T>)
        {
            static const std::map<std::string, int> map = []<int ...I>(std::integer_sequence<int, I...>)
            {
                return std::map<std::string, int>{{field_name<T>(I), I}...};
            }
            (std::make_integer_sequence<int, field_count<T>>{});

            if (auto it = map.find(name); it != map.end())
                return it->second;
            else
                return -1;
        }
        else
        {
            return field_index<type_without_cvref>(name);
        }
    }


    // Any category:
    template <typename T> std::string to_string(const T &object)
    {
        static_assert(!is_not_reflected<T>, "The type is not reflected.");

        if constexpr (is_primitive<T>)
        {
            return PrimitiveInterface<T>::to_string(object);
        }
        else // is_structure<T>
        {
            std::string ret = "{";
            cexpr_for<field_count<T>>([&](auto index)
            {
                if constexpr (index.value != 0)
                    ret += ",";
                ret += field_name<T>(index.value);
                ret += "=";
                ret += Refl::to_string(field<index.value>(object)); // Refl:: is there to prevent std::to_string from being choosen by ADL.
            });
            ret += "}";
            return ret;
        }
    }

    struct ParseException : std::exception
    {
        std::string message;
        std::string stack;
        int line = 0, column = 0;

        ParseException() {}
        ParseException(std::string message) : message(message) {}

        mutable std::string description; // This is created by calling .what();

        const char *what() const noexcept override // This generates `description` (only if it's still empty) and returns its `.c_str()`.
        {
            if (description.empty())
            {
                if (line != 0)
                    description += "At " + std::to_string(line) + ":" + std::to_string(column) + ": ";
                if (stack.size() > 0)
                    description += "At " + stack + ": ";
                description += message;
            }

            return description.c_str();
        }
    };

    namespace impl
    {
        enum class IgnoreMissingFields {no, yes};
        enum class IgnoreIncompleteRefl {no, yes};

        // On failure, throws ParseException (without line and column information), leaves `object` in an unspecified state, and leaves `string` pointing to the error location.
        template <IgnoreMissingFields ignore_missing_fields, IgnoreIncompleteRefl ignore_incomplete_refl, typename T>
        void from_string_low(T &object, const char *&string)
        {
            static_assert(!std::is_const_v<T>, "The type has to be non-const.");
            static_assert(!is_not_reflected<T>, "The type is not reflected.");

            // Skip initial whitespace.
            skip_whitespace(string);

            if constexpr (is_primitive<T>)
            {
                if (!PrimitiveInterface<T>::from_string(object, string))
                    throw ParseException("Unable to parse.");
            }
            else // is_structure<T>
            {
                // Build an array of function pointers for parsing fields.
                using this_func_t = void(T &, const char *&);

                constexpr std::array<this_func_t *, field_count<T>> field_parse_functions = []<int ...I>(std::integer_sequence<int, I...>)
                {
                    return std::array<this_func_t *, field_count<T>>
                    {
                        [](T &object, const char *&string)
                        {
                            from_string_low<ignore_missing_fields, ignore_incomplete_refl>(field<I>(object), string);
                        }...
                    };
                }
                (std::make_integer_sequence<int, field_count<T>>{});

                // Skip the `{`.
                if (*string != '{') throw ParseException("Expected `{`.");
                string++;

                // Skip the whitespace between `{` and the first field name.
                skip_whitespace(string);

                bool first = 1;

                while (1)
                {
                    // Stop if encountered `}`.
                    if (*string == '}')
                    {
                        string++;
                        break;
                    }

                    // Skip `,` between fields.
                    if (first)
                    {
                        first = 0;
                    }
                    else
                    {
                        if (*string != ',') throw ParseException("Expected `,` or `}`.");
                        string++;

                        // Skip whitespace between `,` and the next field name or `}`.
                        skip_whitespace(string);
                    }

                    // Once again, stop if encountered `}`.
                    if (*string == '}')
                    {
                        string++;
                        break;
                    }

                    // Read field name.
                    std::string name;
                    while (is_alphanum(*string))
                        name += *string++;

                    // Stop if the field name is empty.
                    if (name.empty()) throw ParseException("Expected field name.");

                    // Obtain field index from name, stop if there is no such field.
                    int index = field_index<T>(name);
                    if (index == -1) throw ParseException("No field named `" + name + "`.");

                    // Skip whitespace between the field name and `=`.
                    skip_whitespace(string);

                    // Skip `=`.
                    if (*string != '=') throw ParseException("Expected `=`.");
                    string++;

                    try
                    {
                        // Try parsing the field.
                        // The whitespace between `=` and the field data is skipped automatically.
                        field_parse_functions[index](object, string);
                    }
                    catch (ParseException &e)
                    {
                        // If parsing fails, append the field name to to the error stack and rethrow the exception.
                        if (e.stack.empty())
                            e.stack = field_name<T>(index);
                        else
                            e.stack = field_name<T>(index) + '.' + e.stack;
                        throw;
                    }

                    // Skip whitespace between the field data and `}` or `,`.
                    skip_whitespace(string);
                }
            }
        }
    }

    template <typename T> void patch_from_string(T &object, const char *string, const char **end = 0)
    {
        const char *begin = string;
        try
        {
            impl::from_string_low<impl::IgnoreMissingFields::yes, impl::IgnoreIncompleteRefl::yes>(object, string);

            if (end)
            {
                *end = string;
            }
            else
            {
                impl::skip_whitespace(string);
                if (*string) throw ParseException("Extra data at the end of string.");
            }
        }
        catch (ParseException &e)
        {
            e.line = 1;
            e.column = 1;
            for (const char *ptr = begin; ptr != string; ptr++)
            {
                if (*ptr == '\n')
                {
                    e.line++;
                    e.column = 1;
                }
                else
                {
                    e.column++;
                }
            }
            throw;
        }
    }
}

#define Reflect(...)                                                                                                                                                                                  \
    constexpr auto _refl_fields_primary() {return ::std::tie(__VA_ARGS__);}                                                                                                                           \
    inline static constexpr auto /* std::array<char,N> */ _refl_field_name_storage_primary = ::Refl::impl::zero_out_delimiters(#__VA_ARGS__);                                                         \
    inline static constexpr int _refl_field_count_primary = ::Refl::impl::count_commas(#__VA_ARGS__) + 1;                                                                                             \
    inline static constexpr auto /* std::array<const char *, N> */ _refl_field_names_primary = ::Refl::impl::extract_field_names<_refl_field_count_primary>(_refl_field_name_storage_primary.data()); \

#define ReflectOptional(...)                                                                                                                                                                             \
    constexpr auto _refl_fields_optional() {return ::std::tie(__VA_ARGS__);}                                                                                                                             \
    inline static constexpr auto /* std::array<char,N> */ _refl_field_name_storage_optional = ::Refl::impl::zero_out_delimiters(#__VA_ARGS__);                                                           \
    inline static constexpr int _refl_field_count_optional = ::Refl::impl::count_commas(#__VA_ARGS__) + 1;                                                                                               \
    inline static constexpr auto /* std::array<const char *, N> */ _refl_field_names_optional = ::Refl::impl::extract_field_names<_refl_field_count_optional>(_refl_field_name_storage_optional.data()); \

#endif
