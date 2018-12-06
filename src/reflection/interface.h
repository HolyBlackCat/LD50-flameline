#pragma once

#include <array>
#include <cstddef>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "program/errors.h"
#include "utils/meta.h"

namespace Refl
{
    enum class FieldCategory {mandatory, optional, default_category = mandatory};
    enum FromStringMode {full, partial};

    namespace Custom
    {
        template <typename T> using not_specialized_tag = typename T::not_specialized_tag;
        template <typename T> inline constexpr bool is_specialized = !Meta::is_detected<not_specialized_tag, T>;

        template <typename T> using structure_tuple_tag = typename T::structure_tuple_tag;
        template <typename T> inline constexpr bool is_structure_tuple = !Meta::is_detected<structure_tuple_tag, T>;

        // For any of those templates, T will never be cv-qualified.

        template <typename T, typename = void> struct Primitive
        {
            using not_specialized_tag = void; // Don't forget to remove this when specializing.

            inline static const std::string name = "??";
            static std::string to_string(const T &) {return "??";}

            // On success, returns 1, changes the referenced object, and advances the char pointer.
            // On failure, returns 0. Values of the object and the char pointer are unspecfified.
            static bool from_string(T &, const char *&) {return 0;}
        };

        template <typename T, typename = void> struct Structure
        {
            using not_specialized_tag = void; // Don't forget to remove this when specializing.

            inline static const std::string name = "??";

            // Field indices are guaranteed to be in valid range.
            static constexpr bool is_tuple = 0; // Forces `to_string` and `from_string` to use a terse syntax without field names (in this case field categories are ignored, all fields are considered mandatory).
            static constexpr int field_count = 0;
            template <int I> static constexpr void field(T &); // When specialized, should return `auto &`.
            static std::string field_name(int index) {(void)index; return "??";}
            static constexpr FieldCategory field_category(int index) {(void)index; return FieldCategory::default_category;}
        };

        template <typename T, typename = void> struct Container
        {
            using not_specialized_tag = void; // Don't forget to remove this when specializing.

            inline static const std::string name = "??";

            static constexpr std::size_t size(const T &) {return 0;}

            // Those should return iterators.
            static constexpr void begin(T &);
            static constexpr void end(T &);
            static constexpr void const_begin(const T &);
            static constexpr void const_end(const T &);

            using element_type = int;

            // Those return 1 on success.
            static constexpr bool insert(T &, const element_type &);
            static constexpr bool insert_move(T &, element_type &&);
        };
    }

    namespace impl
    {
        template <typename T> struct is_reflected
        {
            static constexpr bool value =
                Custom::is_specialized<Custom::Primitive<std::remove_cv_t<std::remove_reference_t<T>>>> ||
                Custom::is_specialized<Custom::Structure<std::remove_cv_t<std::remove_reference_t<T>>>> ||
                Custom::is_specialized<Custom::Container<std::remove_cv_t<std::remove_reference_t<T>>>>;
        };
        template <> struct is_reflected<void> : std::false_type {}; // We can't test void for reflected-ness by normal means, since instantinating interfaces for void generates hard errors.

        template <typename T> struct make_mutable
        {
            using type = T;
        };
        template <typename A, typename B> struct make_mutable<std::pair<const A, B>>
        {
            using type = std::pair<A, B>;
        };


        inline constexpr bool is_alphanum(char ch)
        {
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_';
        }

        inline void skip_whitespace(const char *&ptr) // Also skips comments (from '#' to '\n' or `\r`).
        {
            while (1)
            {
                while (*ptr > '\0' && *ptr <= ' ')
                    ptr++;

                if (*ptr != '#')
                    return;

                ptr++;
                while (*ptr && *ptr != '\n' && *ptr != '\r')
                    ptr++;
            }
        }
    }

    template <typename T> inline constexpr bool is_reflected = impl::is_reflected<T>::value;

    template <typename T> class Interface
    {
      public:
        using type = std::conditional_t<std::is_reference_v<T>, T, T &>; // This is always a reference, & or && depending on target value category.
        using type_no_ref = std::remove_reference_t<T>;
        using type_no_cvref = std::remove_cv_t<type_no_ref>;

        static constexpr bool is_lvalue = std::is_lvalue_reference_v<type>;
        static constexpr bool is_mutable = !std::is_const_v<type_no_ref>;

        static constexpr bool is_primitive = Custom::is_specialized<Custom::Primitive<type_no_cvref>>;
        static constexpr bool is_structure = Custom::is_specialized<Custom::Structure<type_no_cvref>>;
        static constexpr bool is_container = Custom::is_specialized<Custom::Container<type_no_cvref>>;
        static_assert(is_primitive || is_structure || is_container, "This type is not reflected.");
        static_assert(is_primitive + is_structure + is_container <= 1, "This type has several reflection interfaces.");

      private:
        type_no_cvref *ptr = 0;

        using low = std::conditional_t<is_primitive, Custom::Primitive<type_no_cvref>,
                    std::conditional_t<is_structure, Custom::Structure<type_no_cvref>,
                                                     Custom::Container<type_no_cvref>>>;

      public:
        template <int I> static constexpr auto field_type_tag() // We're unable to make this private.
        {
            if constexpr (!is_structure)
            {
                return Meta::tag<void>{};
            }
            else
            {
                static_assert(I >= 0 && I < field_count(), "Field index is out of range.");
                return Meta::tag<std::remove_reference_t<decltype(low::template field<I>(*(type_no_cvref *)0))>>{};
            }
        }
        static constexpr auto element_type_tag() // We're unable to make this private.
        {
            if constexpr (!is_container)
            {
                return Meta::tag<void>{};
            }
            else
            {
                return Meta::tag<typename low::element_type>{};
            }
        }

        static constexpr bool is_structure_tuple = []() -> bool {
            if constexpr (is_structure)
                return low::is_tuple;
            else
                return 0;
        }();

        constexpr Interface() {};
        constexpr Interface(type_no_ref &ref) : ptr((type_no_cvref *)&ref) {}
        constexpr Interface(type_no_ref &&ref) : ptr((type_no_cvref *)&ref) {}

        // Universal
        constexpr type value() const // Returns a reference to the target object with preserved value category.
        {
            return *ptr;
        }

        static const std::string &type_name()
        {
            return low::name;
        }

        // `indent` is the amount of spaces you want to indent with, or negative value for no indentation and no line breaks at all.
        // `cur_indent` is the base intentation level, measured in spaces. It should be non-negative. If `indent < 0`, it has no effect. It's not applied to the first line.
        std::string to_string(int indent_step = -1, int cur_indent = 0) const
        {
            if constexpr (is_primitive)
            {
                return low::to_string(*ptr);
            }
            else if constexpr (is_structure)
            {
                bool should_indent = indent_step >= 0;
                int next_indent = cur_indent + indent_step;

                std::string ret;

                ret += (!is_structure_tuple ? '{' : '(');

                if constexpr (field_count() > 0)
                {
                    if (!is_structure_tuple && should_indent)
                        ret += '\n';

                    for_each_field([&, this](auto index)
                    {
                        constexpr int i = index.value;

                        if constexpr (i != 0)
                        {
                            ret += ',';

                            if (should_indent)
                                ret += (is_structure_tuple ? ' ' : '\n');
                        }

                        if constexpr (!is_structure_tuple)
                        {
                            if (should_indent)
                                ret += std::string(next_indent, ' ');

                            ret += field_name(i);

                            if (should_indent)
                                ret += ' ';

                            ret += '=';

                            if (should_indent)
                                ret += ' ';
                        }
                        ret += field<i>().to_string(indent_step, next_indent);
                    });

                    if (!is_structure_tuple && should_indent)
                    {
                        ret += ",\n";
                        ret += std::string(cur_indent, ' ');
                    }
                }

                ret += (!is_structure_tuple ? '}' : ')');

                return ret;
            }
            else // is_container
            {
                bool should_indent = indent_step >= 0;
                int next_indent = cur_indent + indent_step;

                std::string ret;

                ret += '[';

                if (size() > 0)
                {
                    if (should_indent)
                        ret += '\n';

                    bool first = 1;
                    for_each_element([&](auto it)
                    {
                        if (first)
                        {
                            first = 0;
                        }
                        else
                        {
                            ret += ',';
                            if (should_indent)
                                ret += '\n';
                        }

                        if (should_indent)
                            ret += std::string(next_indent, ' ');

                        ret += Interface<element_type>(*it).to_string(indent_step, next_indent); // We have to use `Refl::Interface` instead of `Interface` for template argument deduction to work.
                    });

                    if (should_indent)
                    {
                        if (!first)
                            ret += ",\n";
                        ret += std::string(cur_indent, ' ');
                    }
                }

                ret += ']';

                return ret;
            }
        }

        void from_string_low(const char *&str, FromStringMode mode)
        {
            static_assert(is_mutable);

            impl::skip_whitespace(str);

            if constexpr (is_primitive)
            {
                (void)mode;

                const char *old_str = str;
                if (!low::from_string(*ptr, str))
                {
                    str = old_str;
                    Program::Error("Primitive type parsing failed.");
                }
            }
            else if constexpr (is_structure)
            {
                if constexpr (!is_structure_tuple)
                {
                    // Make functions to parse fields.
                    constexpr auto field_parsers = Meta::cexpr_generate_array<field_count()>(
                        [](auto index)
                        {
                            constexpr int i = index.value;

                            return +[](Interface &interface, const char *&str, FromStringMode mode)
                            {
                                interface.field<i>().from_string_low(str, mode);
                            };
                        }
                    );

                    // Flags for existing fields.
                    bool existing_fields[field_count()]{};

                    // Skip the `{`.
                    if (*str != '{') Program::Error("Expected '{'.");
                    str++;

                    // Skip whitespace after `{`.
                    impl::skip_whitespace(str);

                    bool first = 1;

                    while (1)
                    {
                        // Stop if `}`.
                        if (*str == '}')
                        {
                            str++;
                            break;
                        }

                        // Skip `,` between fields.
                        if (first)
                        {
                            first = 0;
                        }
                        else
                        {
                            if (*str != ',') Program::Error("Expected ',' or '}'.");
                            str++;

                            // Skip whitespace after `,`.
                            impl::skip_whitespace(str);
                        }

                        // One more time, stop if `}`.
                        if (*str == '}')
                        {
                            str++;
                            break;
                        }

                        // Read field name.
                        std::string name;
                        while (impl::is_alphanum(*str))
                            name += *str++;

                        // Stop if field name is empty.
                        if (name.empty()) Program::Error("Expected field name.");

                        // Obtain field index.
                        int index = field_index_from_name(name);
                        if (index == -1) Program::Error("No field named `", name, "`.");

                        // Make sure we didn't read the same field before.
                        if (existing_fields[index]) Program::Error("Duplicate field named `", name, "`.");

                        // Skip whitespace after field name.
                        impl::skip_whitespace(str);

                        // Skip `=`.
                        if (*str != '=') Program::Error("Expected '='.");
                            str++;

                        try
                        {
                            // Try parsing the field.
                            field_parsers[index](*this, str, mode);
                        }
                        catch (std::exception &e)
                        {
                            std::string msg = e.what();
                            std::string append;
                            append = "." + name;
                            if (msg[0] != '.')
                                append += ": ";
                            msg = append + msg;
                            Program::Error(msg);
                        }

                        // Mark the field as existing.
                        existing_fields[index] = 1;

                        // Skip whitespace after field.
                        impl::skip_whitespace(str);
                    }

                    // Check for uninitialized fields.
                    if (mode == full)
                    {
                        bool incomplete = 0;
                        std::string missing;
                        for (int i = 0; i < field_count(); i++)
                        {
                            if (field_category(i) == FieldCategory::optional)
                                continue;
                            if (!existing_fields[i])
                            {
                                incomplete = 1;
                                if (missing.size() > 0)
                                    missing += ", ";
                                missing += '`';
                                missing += field_name(i);
                                missing += '`';
                            }
                        }
                        if (incomplete)
                            Program::Error("Following fields are missing: ", missing, ".");
                    }
                }
                else
                {
                    // Skip the `(`.
                    if (*str != '(') Program::Error("Expected '('.");
                    str++;

                    // Skip whitespace after `(`.
                    impl::skip_whitespace(str);

                    for_each_field([&, this](auto index)
                    {
                        constexpr int i = index.value;

                        // Skip `,` between fields.
                        if constexpr (i != 0)
                        {
                            if (*str != ',') Program::Error("Expected ','.");
                            str++;
                        }

                        try
                        {
                            // Try parsing the field.
                            field<i>().from_string_low(str, mode);
                        }
                        catch (std::exception &e)
                        {
                            std::string msg = e.what();
                            std::string append;
                            append = "." + field_name(i) + "(" + std::to_string(i) + ")";
                            if (msg[0] != '.')
                                append += ": ";
                            msg = append + msg;
                            Program::Error(msg);
                        }

                        // Skip whitespace after the field.
                        impl::skip_whitespace(str);
                    });

                    // Skip the trailing comma if we have one.
                    if (*str == ',')
                    {
                        str++;

                        // Skip whitespace after the trailing comma.
                        impl::skip_whitespace(str);
                    }

                    // Skip the `)`.
                    if (*str != ')') Program::Error("Expected ')' or ',)'.");
                    str++;
                }
            }
            else // is_container
            {
                (void)mode;

                // Clear the container.
                *ptr = {};

                // Skip the `[`.
                if (*str != '[') Program::Error("Expected '['.");
                str++;

                // Skip whitespace after `[`.
                impl::skip_whitespace(str);

                int index = 0;

                while (1)
                {
                    // Stop if `]`.
                    if (*str == ']')
                    {
                        str++;
                        break;
                    }

                    // Skip `,` between element.
                    if (index > 0)
                    {
                        if (*str != ',') Program::Error("Expected ',' or ']'.");
                        str++;

                        // Skip whitespace after `,`.
                        impl::skip_whitespace(str);
                    }

                    // One more time, stop if `]`.
                    if (*str == ']')
                    {
                        str++;
                        break;
                    }

                    try
                    {
                        // Try parsing the element.
                        mutable_element_type tmp{};
                        Interface<mutable_element_type>(tmp).from_string_low(str, mode);
                        if (!insert(std::move(tmp)))
                            Program::Error("Invalid element.");
                    }
                    catch (std::exception &e)
                    {
                        std::string msg = e.what();
                        std::string append;
                        append = "." + std::to_string(index);
                        if (msg[0] != '.')
                            append += ": ";
                        msg = append + msg;
                        Program::Error(msg);
                    }

                    // Skip whitespace after element.
                    impl::skip_whitespace(str);

                    // Increment index.
                    index++;
                }
            }
        }
        void from_string(const std::string &str, FromStringMode mode = full)
        {
            const char *ptr = str.c_str();
            try
            {
                from_string_low(ptr, mode);
                impl::skip_whitespace(ptr);
                if (str.c_str() + str.size() != ptr)
                    Program::Error("Unexpected data at the end of string.");
            }
            catch (std::exception &e)
            {
                std::string msg = e.what();
                if (msg[0] == '.')
                {
                    msg.erase(msg.begin());
                    msg = "At " + msg;
                }
                msg = "Unable to parse reflected object:\n" + msg;
                Program::Error(msg);
            }
        }

        template <typename F> static constexpr void for_each_field(F &&func) // `func` receives indices as `std::integral_constant<int,i>`.
        {
            static_assert(is_structure);
            Meta::cexpr_for<field_count()>(std::forward<F>(func));
        }

        // Structure-specific
        static constexpr int field_count()
        {
            static_assert(is_structure);
            return low::field_count;
        }
        template <int I> using field_type = typename decltype(field_type_tag<I>())::type;
        template <int I> constexpr decltype(auto) field_value() const
        {
            static_assert(is_structure);
            static_assert(I >= 0 && I < field_count(), "Field index is out of range.");
            auto &ref = low::template field<I>(*ptr);
            using field_type = std::remove_reference_t<decltype(ref)>;
            using field_type_cv = std::conditional_t<is_mutable, field_type, const field_type>;
            using field_type_cvref = std::conditional_t<is_lvalue, field_type_cv &, field_type_cv &&>;
            return field_type_cvref(ref);
        }
        template <int I> constexpr auto field() const
        {
            static_assert(is_structure);
            return Interface<decltype(field_value<I>())>(field_value<I>()); // We have to use `Refl::Interface` instead of `Interface` for template argument deduction to work.
        }
        static std::string field_name(int index)
        {
            static_assert(is_structure);
            if (index < 0 || index >= field_count())
                return "N/A";
            return low::field_name(index);
        }
        static constexpr FieldCategory field_category(int index)
        {
            static_assert(is_structure);
            if (index < 0 || index >= field_count())
                return FieldCategory::default_category;
            return low::field_category(index);
        }
        static int field_index_from_name(std::string name) // Returns -1 if no such field.
        {
            if constexpr (!is_lvalue || !is_mutable)
            {
                return Interface<type_no_cvref &>::field_index_from_name(name);
            }
            else
            {
                static const auto map = Meta::cexpr_generate<std::map<std::string, int>, field_count()>(
                    [](auto index)
                    {
                        constexpr int i = index.value;
                        return std::pair{field_name(i), i};
                    }
                );

                if (auto it = map.find(name); it != map.end())
                    return it->second;
                else
                    return -1;
            }
        }

        // Container-specific
        using element_type = typename decltype(element_type_tag())::type;
        using mutable_element_type = typename impl::make_mutable<std::remove_const_t<element_type>>::type;
        std::size_t size() const
        {
            static_assert(is_container);
            return low::size(*ptr);
        }
        bool empty() const
        {
            static_assert(is_container);
            return size() == 0;
        }
        auto begin() const
        {
            static_assert(is_container);
            if constexpr (is_mutable)
                return low::begin(*ptr);
            else
                return low::const_begin(*ptr);
        }
        auto end() const
        {
            static_assert(is_container);
            if constexpr (is_mutable)
                return low::end(*ptr);
            else
                return low::const_end(*ptr);
        }
        template <typename F> void for_each_element(F &&func) const // `func` receives iterators.
        {
            static_assert(is_container);
            for (auto it = begin(); it != end(); ++it)
                func(std::as_const(it));
        }
        template <typename A> bool insert(A &&value) // Returns 1 on success.
        {
            static_assert(is_container);
            if constexpr (std::is_reference_v<A>)
                return low::insert(*ptr, value);
            else
                return low::insert_move(*ptr, std::move(value));
        }
    };

    template <typename T> Interface(T &) -> Interface<T &>;
    template <typename T> Interface(T &&) -> Interface<T &&>;
}
