#ifndef REFLECTION_INTERFACE_H_INCLUDED
#define REFLECTION_INTERFACE_H_INCLUDED

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "utils/meta.h"

namespace Refl
{
    enum class FieldCategory {mandatory, optional, default_category = mandatory};

    namespace Custom
    {
        template <typename T> using not_specialized_tag = typename T::not_specialized_tag;
        template <typename T> inline constexpr bool is_specialized = !Meta::is_detected<not_specialized_tag, T>;

        // For any of those templates, T will never be cv-qualified.

        template <typename T, typename = void> struct Primitive
        {
            using not_specialized_tag = void; // Don't forget to remove this when specializing.

            static constexpr const char *name = "??";
            static std::string to_string(const T &) {return "??";}
        };

        template <typename T, typename = void> struct Structure
        {
            using not_specialized_tag = void; // Don't forget to remove this when specializing.

            // Field indices are guaranteed to be in valid range.
            static constexpr const char *name = "??";
            static constexpr int field_count = 0;
            template <int I> static constexpr void field(T &); // When specialized, should return `auto &`.
            static std::string field_name(int index) {(void)index; return "??";}
            static constexpr FieldCategory field_category(int index) {(void)index; return FieldCategory::default_category;}
        };

        template <typename T, typename = void> struct Container
        {
            using not_specialized_tag = void; // Don't forget to remove this when specializing.

            static constexpr const char *name = "??";

            static constexpr std::size_t size(const T &) {return 0;}

            // Those should return iterators.
            static constexpr void begin(T &) {}
            static constexpr void end(T &) {}
            static constexpr void const_begin(const T &) {}
            static constexpr void const_end(const T &) {}
        };
    }


    template <typename T> class Interface
    {
        using type = std::conditional_t<std::is_reference_v<T>, T, T &>;
        using type_no_ref = std::remove_reference_t<T>;
        using type_no_cvref = std::remove_cv_t<type_no_ref>;

      type_no_cvref *ptr = 0;

      public:
        static constexpr bool is_lvalue = std::is_lvalue_reference_v<type>;
        static constexpr bool is_mutable = !std::is_const_v<type_no_ref>;

        static constexpr bool is_primitive = Custom::is_specialized<Custom::Primitive<type_no_cvref>>;
        static constexpr bool is_structure = Custom::is_specialized<Custom::Structure<type_no_cvref>>;
        static constexpr bool is_container = Custom::is_specialized<Custom::Container<type_no_cvref>>;
        static_assert(is_primitive || is_structure || is_container, "This type is not reflected.");
        static_assert(is_primitive + is_structure + is_container <= 1, "This type has several reflection categories.");

      private:
        using impl = std::conditional_t<is_primitive, Custom::Primitive<type_no_cvref>,
                     std::conditional_t<is_structure, Custom::Structure<type_no_cvref>,
                                                      Custom::Container<type_no_cvref>>>;

      public:
        constexpr Interface() {};
        constexpr Interface(type_no_ref &ref) : ptr((type_no_cvref *)&ref) {}
        constexpr Interface(type_no_ref &&ref) : ptr((type_no_cvref *)&ref) {}

        constexpr type value() const // Returns the target object with preserved value category.
        {
            return *ptr;
        }

        static std::string type_name()
        {
            return impl::name;
        }

        // Universal
        std::string to_string() const
        {
            if constexpr (is_primitive)
            {
                return impl::to_string(*ptr);
            }
            else if constexpr (is_structure)
            {
                std::string ret;
                ret += '{';
                for_each_field([&, this](auto index)
                {
                    constexpr int i = index.value;

                    if constexpr (i != 0)
                        ret += ',';

                    ret += field_name(i);
                    ret += '=';
                    ret += field<i>().to_string();
                });
                ret += '}';
                return ret;
            }
            else // is_container
            {
                std::string ret;
                ret += '[';
                bool first = 1;
                for_each_element([&](auto it)
                {
                    if (first)
                        first = 0;
                    else
                        ret += ',';

                    ret += Refl::Interface(*it).to_string(); // We have to use `Refl::Interface` instead of `Interface` for template argument deduction to work.
                });
                ret += ']';
                return ret;
            }
        }
        template <typename F> constexpr void for_each_field(F &&func) const // Func receives indices as `std::integral_constant<int,i>`.
        {
            static_assert(is_structure);
            Meta::cexpr_for<field_count()>(std::forward<F>(func));
        }

        // Structure-specific
        static constexpr int field_count()
        {
            static_assert(is_structure);
            return impl::field_count;
        }
        template <int I> constexpr decltype(auto) field_value() const
        {
            static_assert(is_structure);
            static_assert(I >= 0 && I < field_count(), "Field index is out of range.");
            auto &ref = impl::template field<I>(*ptr);
            using field_type = std::remove_reference_t<decltype(ref)>;
            using field_type_cv = std::conditional_t<is_mutable, field_type, const field_type>;
            using field_type_cvref = std::conditional_t<is_lvalue, field_type_cv &, field_type_cv &&>;
            return field_type_cvref(ref);
        }
        template <int I> constexpr auto field() const
        {
            static_assert(is_structure);
            return Refl::Interface(field_value<I>()); // We have to use `Refl::Interface` instead of `Interface` for template argument deduction to work.
        }
        static std::string field_name(int index)
        {
            static_assert(is_structure);
            if (index < 0 || index >= field_count())
                return "N/A";
            return impl::field_name(index);
        }
        static constexpr FieldCategory field_category(int index)
        {
            static_assert(is_structure);
            if (index < 0 || index >= field_count())
                return FieldCategory::default_category;
            return impl::field_category(index);
        }

        // Container-specific
        constexpr auto begin() const
        {
            static_assert(is_container);
            if constexpr (is_mutable)
                return impl::begin(*ptr);
            else
                return impl::const_begin(*ptr);
        }
        constexpr auto end() const
        {
            static_assert(is_container);
            if constexpr (is_mutable)
                return impl::end(*ptr);
            else
                return impl::const_end(*ptr);
        }
        template <typename F> void for_each_element(F &&func) const // Func receives iterators.
        {
            static_assert(is_container);
            for (auto it = begin(); it != end(); ++it)
                func(std::as_const(it));
        }
    };

    template <typename T> Interface(T &) -> Interface<T &>;
    template <typename T> Interface(T &&) -> Interface<T &&>;
}

#endif
