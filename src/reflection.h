#ifndef REFLECTION_H_INCLUDED
#define REFLECTION_H_INCLUDED

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>

namespace Refl
{
    namespace impl
    {
        inline namespace Strings
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

            constexpr bool is_alphanum(char ch)
            {
                return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
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

        template <typename T, typename = void> struct is_macro_reflected_impl : std::false_type {};
        template <typename T> struct is_macro_reflected_impl<T, std::void_t<typename T::_refl_tag>> : std::true_type {};
        template <typename T> inline constexpr bool is_macro_reflected_v = is_macro_reflected_impl<T>::value;
    }

    namespace Custom // Customization points
    {
        template <typename T, typename = void> struct Structure // T is never cv-qualified.
        {
            static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "Internal error: T must not be cv-qualified at this point.");

            static constexpr int field_count()
            {
                if constexpr (impl::is_macro_reflected_v<T>)
                    return T::_refl_field_count;
                else
                    return 0;
            }

            static constexpr const char *field_name(int index) // `index` will never be out of range
            {
                static_assert(impl::is_macro_reflected_v<T>, "Field names are not reflected for this type .");
                return T::_refl_field_names[index];
            }

            template <int I> static constexpr auto &field_lvalue_ref(T &object) // `I` will never be out of range
            {
                static_assert(impl::is_macro_reflected_v<T>, "Fields are not reflected for this type .");
                return std::get<I>(object._refl_fields());
            }
        };

//        template <typename T, typename = void> struct Primitive
//        {
//            static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "Internal error: T must not be cv-qualified at this point.");
//        };
    }

    template <typename T> inline constexpr bool is_structure = Custom::Structure<std::remove_cv_t<T>>::field_count() > 0;
    template <typename T> inline constexpr bool is_primitive = !is_structure<T>;


    template <typename T> inline constexpr int field_count = Custom::Structure<std::remove_cv_t<T>>::field_count();

    template <typename T> std::string field_name(int index)
    {
        if (index >= 0 && index < field_count<T>)
            return Custom::Structure<std::remove_cv_t<T>>::field_name(index);
        else
            return std::to_string(index);
    }

    template <int I, typename T> decltype(auto) field(T &&object)
    {
        using param_base_t = std::remove_cv_t<std::remove_reference_t<T>>;
        static_assert(is_structure<param_base_t>, "The type is not a reflected structure.");
        static_assert(I < field_count<param_base_t> && I >= 0, "Field index is out of range.");
        constexpr bool is_mutable = !std::is_const_v<std::remove_reference_t<T>>;
        constexpr bool is_lvalue = std::is_reference_v<T>;
        using result_base_t = std::remove_reference_t<decltype(Custom::Structure<param_base_t>::template field_lvalue_ref<I>(std::declval<param_base_t &>()))>; // This type may be already cv-qualified.
        using result_cv_t = std::conditional_t<is_mutable, result_base_t, const result_base_t>;
        using result_t = std::conditional_t<is_lvalue, result_cv_t &, result_cv_t &&>;
        return static_cast<result_t>(Custom::Structure<param_base_t>::template field_lvalue_ref<I>(const_cast<param_base_t &>(object)));
    }
}

#define Reflect(...)                                                                                                                                                                \
    using _refl_tag = void;                                                                                                                                                         \
    constexpr auto _refl_fields() {return ::std::tie(__VA_ARGS__);}                                                                                                                 \
    inline static constexpr auto /* std::array<char,N> */ _refl_field_name_storage = ::Refl::impl::zero_out_delimiters(#__VA_ARGS__);                                         \
    inline static constexpr int _refl_field_count = ::Refl::impl::count_commas(#__VA_ARGS__) + 1;                                                                             \
    inline static constexpr auto /* std::array<const char *, N> */ _refl_field_names = ::Refl::impl::extract_field_names<_refl_field_count>(_refl_field_name_storage.data()); \

#endif
