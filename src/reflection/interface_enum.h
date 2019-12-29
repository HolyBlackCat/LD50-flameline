#pragma once

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <type_traits>
#include <vector>

#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "reflection/interface_scalar.h"
#include "utils/lexical_cast.h"
#include "utils/macro.h"
#include "utils/meta.h"
#include "utils/named_macro_parameters.h"

namespace Refl
{
    namespace impl::Enum
    {
        template <typename T>
        struct ValueNamePair
        {
            T value = {};
            const char *name = nullptr;

            bool operator<(const ValueNamePair &other) const
            {
                using underlying = std::underlying_type_t<T>;
                return underlying(value) < underlying(other.value);
            }
        };

        template <typename T>
        struct NameValuePair
        {
            const char *name = {};
            T value = nullptr;

            bool operator<(const NameValuePair &other) const
            {
                return std::strcmp(name, other.name) < 0;
            }
        };

        template <typename T>
        class Helper
        {
            using underlying = std::underlying_type_t<T>;

            std::vector<ValueNamePair<T>> values_to_names;
            std::vector<NameValuePair<T>> names_to_values;
            bool is_relaxed = false;

          public:
            Helper(std::initializer_list<ValueNamePair<T>> list, bool is_relaxed)
                : is_relaxed(is_relaxed)
            {
                values_to_names = std::vector<ValueNamePair<T>>(list.begin(), list.end());
                std::sort(values_to_names.begin(), values_to_names.end());

                names_to_values.reserve(list.size());
                for (const auto &elem : list)
                    names_to_values.push_back({elem.name, elem.value});
                std::sort(names_to_values.begin(), names_to_values.end());
            }

            Helper(const Helper &) = delete;
            Helper &operator=(const Helper &) = delete;

            // If this returns true, the enum is considered to be 'relaxed'.
            // When serializing, if `ValueToName` fails you should write the numeric value.
            // When deserializing, if `NameToValue` fails you should attempt to read the numeric value.
            bool IsRelaxed() const
            {
                return is_relaxed;
            }

            // Returns nullptr on failure.
            const char *ValueToName(T value) const
            {
                ValueNamePair<T> dummy{value, nullptr};
                auto it = std::lower_bound(values_to_names.begin(), values_to_names.end(), dummy);
                if (it == values_to_names.end() || it->value != value)
                    return nullptr;
                return it->name;
            }

            // Returns T{} on failure. If `ok` is not null, it's set to true on success or to false on failure.
            T NameToValue(const char *name, bool *ok = nullptr) const
            {
                NameValuePair<T> dummy{name, {}};
                auto it = std::lower_bound(names_to_values.begin(), names_to_values.end(), dummy);
                if (it == names_to_values.end() || std::strcmp(it->name, name) != 0)
                {
                    if (ok)
                        *ok = false;
                    return {};
                }
                if (ok)
                    *ok = true;
                return it->value;
            }
        };

        void zrefl_EnumFunc() = delete; // Dummy ADL target.

        template <typename T>
        auto GetHelper() -> const decltype(zrefl_EnumFunc(std::declval<T>()).helper) &
        {
            return zrefl_EnumFunc(T{}).helper;
        }

        template <typename T> using detect_enum = decltype(impl::Enum::GetHelper<T>());
    }

    template <typename T>
    class Interface_Enum : public InterfaceBasic<T>
    {
        using underlying = std::underlying_type_t<T>;

      public:
        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options) const override
        {
            (void)options;
            const auto &helper = impl::Enum::GetHelper<T>();

            const char *name = helper.ValueToName(object);
            if (!name)
            {
                if (!helper.IsRelaxed())
                    Program::Error(output.GetExceptionPrefix(), "Unable to serialize enum: Invalid value: ", underlying(object), ".");
                Interface<underlying>().ToString(underlying(object), output, options);
                return;
            }
            output.WriteString(name);
        }

        void FromString(T &object, Stream::Input &input, const FromStringOptions &options) const override
        {
            (void)options;
            const auto &helper = impl::Enum::GetHelper<T>();

            if (helper.IsRelaxed())
            {
                char ch = input.PeekChar();
                if (Stream::Char::IsDigit{}(ch) || ch == '+' || ch == '-')
                {
                    underlying value = 0;
                    Interface<underlying>().FromString(value, input, options);
                    object = T(value);
                    return;
                }
            }

            std::string name = input.Extract(Stream::Char::SeqIdentifier{});
            bool name_ok = false;
            T result = helper.NameToValue(name.c_str(), &name_ok);
            if (!name_ok)
                Program::Error(input.GetExceptionPrefix(), "Unknown enumerator: `", name, "`.");
            object = result;
        }

        void ToBinary(const T &object, Stream::Output &output) const override
        {
            const auto &helper = impl::Enum::GetHelper<T>();
            if (!helper.IsRelaxed() && helper.ValueToName(object) == nullptr)
                Program::Error(output.GetExceptionPrefix(), "Unable to serialize enum: Invalid value: ", underlying(object), ".");

            Interface<underlying>().ToBinary(underlying(object), output);
        }

        void FromBinary(T &object, Stream::Input &input, const FromBinaryOptions &options) const override
        {
            const auto &helper = impl::Enum::GetHelper<T>();

            underlying result = 0;
            Interface<underlying>().FromBinary(result, input, options);

            if (!helper.IsRelaxed() && helper.ValueToName(T(result)) == nullptr)
                Program::Error(input.GetExceptionPrefix(), "Invalid enum value: ", result, ".");

            object = T(result);
        }
    };

    template <typename T>
    struct impl::SelectInterface<T, Meta::void_type<impl::Enum::detect_enum<T>>>
    {
        using type = Interface_Enum<T>;
    };

    template <typename T>
    struct impl::HasShortStringRepresentation<T, Meta::void_type<impl::Enum::detect_enum<T>>> : std::true_type {};
}


// This enum is a `enum class`.
#define REFL_ENUM_CLASS MA_PARAM(ReflIsClass)
#define MA_PARAMS_category_ReflEnum_X_ReflIsClass
#define MA_PARAMS_equal_ReflIsClass_X_ReflIsClass

// This enum is 'relaxed'.
// Saving and loading unnamed numeric values is allowed.
#define REFL_ENUM_RELAXED MA_PARAM(ReflIsRelaxed)
#define MA_PARAMS_category_ReflEnum_X_ReflIsRelaxed
#define MA_PARAMS_equal_ReflIsRelaxed_X_ReflIsRelaxed

// Allows you to specify an underlying type for a enum.
#define REFL_ENUM_TYPE MA_PARAM(ReflUnderlyingType)
#define MA_PARAMS_category_ReflEnum_X_ReflUnderlyingType
#define MA_PARAMS_equal_ReflUnderlyingType_X_ReflUnderlyingType

/* Declares a reflected enum.
 * Usage:
 *   REFL_ENUM( Name [REFL_ENUM_CLASS] [REFL_ENUM_RELAXED] [REFL_ENUM_TYPE Type] ,
 *       ( Name1 [ , value1] )
 *       ( Name2 [ , value2] )
 *   )
 * Optional parameters following the enum name can be reordered.
 * Parameter meaning is described above.
 */
#define REFL_ENUM(name_params, seq) \
    REFL_ENUM_impl( \
        MA_PARAMS_FIRST(((name_params))), \
        MA_PARAMS_GET_ONE(, ReflEnum, ReflUnderlyingType, ((name_params)), REFL_ENUM_impl_type), \
        MA_PARAMS_GET_ONE(, ReflEnum, ReflIsClass, ((name_params)), MA_PARAMS_DUMMY_EMPTY), \
        seq \
    ) \
    REFL_ENUM_METADATA( \
        MA_PARAMS_FIRST(((name_params))), \
        MA_IF_NOT_EMPTY_ELSE(true, false, MA_PARAMS_GET_ONE(, ReflEnum, ReflIsRelaxed, ((name_params)), MA_PARAMS_DUMMY_EMPTY)), \
        seq \
    )

#define REFL_ENUM_impl(name_, maybe_type_, is_class_if_not_empty_, seq_) \
    enum MA_IF_NOT_EMPTY(class, is_class_if_not_empty_) name_ maybe_type_ { REFL_ENUM_impl_elem_loop(seq_) };

#define REFL_ENUM_impl_type(data, type) : type

#define REFL_ENUM_impl_elem_loop(seq_) MA_APPEND_TO_VA_END(_end, REFL_ENUM_impl_elem_loop_a seq_)
#define REFL_ENUM_impl_elem_loop_a(...) REFL_ENUM_impl_elem(__VA_ARGS__) REFL_ENUM_impl_elem_loop_b
#define REFL_ENUM_impl_elem_loop_b(...) REFL_ENUM_impl_elem(__VA_ARGS__) REFL_ENUM_impl_elem_loop_a
#define REFL_ENUM_impl_elem_loop_a_end
#define REFL_ENUM_impl_elem_loop_b_end
#define REFL_ENUM_impl_elem(...) MA_CALL(REFL_ENUM_impl_elem0,__VA_ARGS__,)
#define REFL_ENUM_impl_elem0(elem_, ...) elem_ MA_IF_NOT_EMPTY(= __VA_ARGS__, __VA_ARGS__),

// Generates metadata for an existing enum.
// Has to be in the same namespace/class as the enum.
// `name_` is the enum type.
// `is_relaxed_` is either `true` or `false`, see `REFL_ENUM_RELAXED` for an explanation.
// `seq_` is a list of constants, see `REFL_ENUM` for explanation.
#define REFL_ENUM_METADATA(name_, is_relaxed_, seq_) \
    struct MA_CAT(zrefl_EnumHelper_, name_) \
    { \
        inline static auto helper = ::Refl::impl::Enum::Helper<name_>({ REFL_ENUM_impl_pair_loop(seq_) }, is_relaxed_); \
    }; \
    [[maybe_unused]] inline static MA_CAT(zrefl_EnumHelper_, name_) zrefl_EnumFunc(name_) {return {};}

#define REFL_ENUM_impl_pair_loop(seq_) MA_APPEND_TO_VA_END(_end, REFL_ENUM_impl_pair_loop_a seq_)
#define REFL_ENUM_impl_pair_loop_a(...) REFL_ENUM_impl_pair(__VA_ARGS__) REFL_ENUM_impl_pair_loop_b
#define REFL_ENUM_impl_pair_loop_b(...) REFL_ENUM_impl_pair(__VA_ARGS__) REFL_ENUM_impl_pair_loop_a
#define REFL_ENUM_impl_pair_loop_a_end
#define REFL_ENUM_impl_pair_loop_b_end
#define REFL_ENUM_impl_pair(...) MA_CALL(REFL_ENUM_impl_pair0,__VA_ARGS__,)
#define REFL_ENUM_impl_pair0(elem_, ...) {type::elem_, #elem_},
