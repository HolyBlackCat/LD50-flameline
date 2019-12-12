#pragma once

#include <cstddef>
#include <string>
#include <type_traits>
#include <string_view>
#include <utility>

#include "reflection/parsing.h"
#include "utils/check.h"
#include "utils/input_stream.h"
#include "utils/meta.h"
#include "utils/output_stream.h"

namespace Refl
{
    struct ToStringOptions
    {
        bool multiline = false;
        int indent = 4; // Indentation step.
        int extra_indent = 0; // Extra indentation at the beginning of each line, except the first one. Intended for the internal use.
    };

    struct FromStringOptions
    {
        // When parsing a struct, don't complain if any fields are missing.
        bool ignore_missing_fields = false;
    };

    struct FromBinaryOptions
    {
        // `reserve()` calls will be capped at this amount of bytes.
        // This prevents malformed serialized data from causing
        // too much temporary memory to be allocated.
        std::size_t max_reserved_size = 1024 * 1024;
    };

    template <typename T>
    class InterfaceBasic : Meta::with_virtual_destructor<InterfaceBasic<T>>
    {
        static_assert(!std::is_reference_v<T>, "Can't make a reflection interface for a reference.");
        static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "Can't make a reflection interface for a type with cv-qualifiers.");

      public:
        constexpr InterfaceBasic() {}

        virtual void ToString(const T &object, Stream::Output &output, const ToStringOptions &options) const = 0;

        // This shouldn't skip any leading or trailing whitespace and comments, and shouldn't check for end of stream.
        virtual void FromString(T &object, Stream::Input &input, const FromStringOptions &options) const = 0;

        virtual void ToBinary(const T &object, Stream::Output &output) const = 0;

        // This shouldn't check for end of stream.
        virtual void FromBinary(T &object, Stream::Input &input, const FromBinaryOptions &options) const = 0;
    };

    namespace impl
    {
        template <typename T, typename = void>
        struct SelectInterface {};
    }


    // Constructs an interface for a type.
    template <typename T>
    auto Interface() -> typename impl::SelectInterface<std::remove_cv_t<T>>::type
    {
        return {};
    }
    template <typename T>
    auto Interface(const T &) -> typename impl::SelectInterface<std::remove_cv_t<T>>::type
    {
        return {};
    }


    inline namespace Shorthands
    {
        // Functions below use this wrapper for extra safety.
        // Otherwise e.g. `foo(ReadOnlydata)` could be called as `foo("bar")`, and would attempt to read `bar` as a file.
        struct ReadOnlyDataWrapper
        {
            Stream::ReadOnlyData value;
            ReadOnlyDataWrapper(Stream::ReadOnlyData value) : value(std::move(value)) {}
        };


        template <typename T, CHECK_EXPR(Interface<T>())>
        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options = {})
        {
            Interface(object).ToString(object, output, options);
        }
        template <typename T, CHECK_EXPR(Interface<T>())>
        [[nodiscard]] std::string ToString(const T &object, const ToStringOptions &options = {})
        {
            std::string ret;
            Stream::Output output = Stream::Output::Container(ret);
            ToString(object, output, options);
            return ret;
        }


        // Skips any leading and trailing whitespace and comments. Expects `input_data` to have no junk at the end.
        template <typename T, CHECK_EXPR(Interface<T>())>
        void FromString(T &object, ReadOnlyDataWrapper input_data, const FromStringOptions &options = {})
        {
            Stream::Input input(std::move(input_data.value), Stream::text_position);
            Parsing::SkipWhitespaceAndComments(input);
            Interface(object).FromString(object, input, options);
            Parsing::SkipWhitespaceAndComments(input);
            input.ExpectEnd();
        }
        template <typename T, CHECK_EXPR(Interface<T>())>
        void FromString(T &object, std::string_view view, const FromStringOptions &options = {})
        {
            FromString(object, Stream::ReadOnlyData::mem_reference(view), options);
        }
        template <typename T, CHECK_EXPR(void(Interface<T>()), T{})>
        [[nodiscard]] T FromString(ReadOnlyDataWrapper input_data, const FromStringOptions &options = {})
        {
            T ret{};
            FromString(ret, std::move(input_data), options);
            return ret;
        }
        template <typename T, CHECK_EXPR(void(Interface<T>()), T{})>
        [[nodiscard]] T FromString(std::string_view view, const FromStringOptions &options = {})
        {
            T ret{};
            FromString(ret, view, options);
            return ret;
        }

        template <typename T, CHECK_EXPR(Interface<T>())>
        void ToBinary(const T &object, Stream::Output &output)
        {
            Interface(object).ToBinary(object, output);
        }

        // Expects `input_data` to have no junk at the end.
        template <typename T, CHECK_EXPR(Interface<T>())>
        void FromBinary(T &object, ReadOnlyDataWrapper input_data, const FromBinaryOptions &options = {})
        {
            Stream::Input input(std::move(input_data.value), Stream::byte_offset);
            Interface(object).FromBinary(object, input, options);
            input.ExpectEnd();
        }
        template <typename T, CHECK_EXPR(void(Interface<T>()), T{})>
        [[nodiscard]] T FromBinary(ReadOnlyDataWrapper input_data, const FromBinaryOptions &options = {})
        {
            T ret{};
            FromBinary(ret, std::move(input_data), options);
            return ret;
        }
    }
}
