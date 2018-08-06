#ifndef REFLECTION_INTERFACE_H_INCLUDED
#define REFLECTION_INTERFACE_H_INCLUDED

#include <tuple>
#include <type_traits>

#include "utils/meta.h"

namespace Refl
{
    enum class FieldCategory {mandatory, optional, default_category = mandatory};

    namespace Custom
    {
        template <typename T> using not_specialized_tag = typename T::not_specialized_tag;
        template <typename T> inline constexpr bool is_specialized = !Meta::is_detected<not_specialized_tag, T>;

        // For any of those templates, T will never be cv-qualified.

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
    }

    inline namespace Interface
    {
        // `T` means same thing as `T &`.

        template <typename T> class Structure
        {
            using type = std::conditional_t<std::is_reference_v<T>, T, T &>;
            using type_no_ref = std::remove_reference_t<T>;
            using type_no_cvref = std::remove_cv_t<type_no_ref>;

            using impl = Custom::Structure<type_no_cvref>;
            static_assert(Custom::is_specialized<impl>, "This type is not a structure.");

            type_no_cvref *ptr = 0;
          public:
            static constexpr bool is_lvalue = std::is_lvalue_reference_v<type>;
            static constexpr bool is_mutable = !std::is_const_v<type_no_ref>;

            constexpr Structure() {};
            constexpr Structure(type_no_ref &ref) : ptr((type_no_cvref *)&ref) {}
            constexpr Structure(type_no_ref &&ref) : ptr((type_no_cvref *)&ref) {}

            constexpr type value() const // Returns the target object with preserved value category.
            {
                return *ptr;
            }

            static constexpr std::string type_name()
            {
                return impl::name;
            }

            static constexpr int field_count()
            {
                return impl::field_count;
            }

            template <int I> constexpr decltype(auto) field() const
            {
                static_assert(I >= 0 && I < field_count(), "Field index is out of range.");
                auto &ref = impl::template field<I>(*ptr);
                using field_type = std::remove_reference_t<decltype(ref)>;
                using field_type_cv = std::conditional_t<is_mutable, field_type, const field_type>;
                using field_type_cvref = std::conditional_t<is_lvalue, field_type_cv &, field_type_cv &&>;
                return field_type_cvref(ref);
            }

            static std::string field_name(int index)
            {
                if (index < 0 || index >= field_count())
                    return "N/A";
                return impl::field_name(index);
            }

            static constexpr FieldCategory field_category(int index)
            {
                if (index < 0 || index >= field_count())
                    return FieldCategory::default_category;
                return impl::field_category(index);
            }
        };

        template <typename T> Structure(T &) -> Structure<T &>;
        template <typename T> Structure(T &&) -> Structure<T &&>;
    }
}

#endif
