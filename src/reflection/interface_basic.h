#pragma once

#include <cstddef>
#include <optional>
#include <string_view>
#include <string>
#include <type_traits>
#include <utility>

#include "macros/check.h"
#include "meta/misc.h"
#include "reflection/utils.h"
#include "stream/input.h"
#include "stream/output.h"

namespace Refl
{
    struct ToStringOptions
    {
        bool pretty = false; // Add extra spaces for readability. Make some containers and structs mulitiline (and add trailing commas to them).
        bool multiline_strings = false; // If enabled, preserve line-feeds when printing `std::string`s. Otherwise they will be printed as `\n`.
        int indent = 4; // Indentation step.

        [[nodiscard]] static ToStringOptions Pretty(int indent = 4)
        {
            ToStringOptions ret;
            ret.pretty = true;
            ret.indent = indent;
            return ret;
        }
    };

    struct FromStringOptions
    {
        // When parsing a struct, don't complain if any fields are missing.
        bool ignore_missing_fields = false;
    };

    struct ToBinaryOptions {};

    struct FromBinaryOptions
    {
        // `reserve()` calls will be capped at this amount of bytes.
        // This prevents malformed serialized data from causing
        // too much temporary memory to be allocated.
        std::size_t max_reserved_size = 1024 * 1024;
    };


    namespace impl
    {
        struct States
        {
            // Those are wrapped in a struct, because we need a way to easily `friend` them.

            template <typename T>
            class Copyable
            {
                T obj;

              public:
                constexpr Copyable(const T &obj) : obj(obj) {}

                [[nodiscard]] constexpr operator T() const
                {
                    return obj;
                }
            };

            template <typename Derived, typename Options>
            class Base // This is a CRTP base.
            {
              protected:
                bool need_virtual_bases = true;
                constexpr Base() {}

                // Moving is same as copying.
                Base(const Base &) = default;
                Base &operator=(const Base &) = default;

              public:
                // A fresh state.
                [[nodiscard]] static constexpr Copyable<Derived> InitialState()
                {
                    return {Derived{}};
                }

                // State for a member or element of the current object.
                // The new object will be intended one level deeper.
                [[nodiscard]] constexpr Copyable<Derived> MemberOrElem(const Options &options) const
                {
                    Derived ret = static_cast<const Derived &>(*this);
                    ret.need_virtual_bases = true;
                    ret.IncreaseNestingLevel(options);
                    return ret;
                }

                // State for a base class of the current object.
                // The new object will be intended one level deeper.
                // Additionally, its virtual base classes will be ignored.
                [[nodiscard]] constexpr Copyable<Derived> BaseClass(const Options &options) const
                {
                    Derived ret = static_cast<const Derived &>(*this);
                    ret.need_virtual_bases = false;
                    ret.IncreaseNestingLevel(options);
                    return ret;
                }

                // State for an artifical object that is used to represent the current object.
                // The indentation for this object will match the current object.
                [[nodiscard]] constexpr Copyable<Derived> PartOfRepresentation(const Options &options) const
                {
                    Derived ret = static_cast<const Derived &>(*this);
                    ret.need_virtual_bases = true;
                    (void)options; // We don't call `IncreaseNestingLevel()` here.
                    return ret;
                }


                [[nodiscard]] constexpr bool NeedVirtualBases() const
                {
                    return need_virtual_bases;
                }
            };
        };

        template <typename Options>
        struct DefaultState : States::Base<DefaultState<Options>, Options>
        {
          private:
            friend States;
            void IncreaseNestingLevel(const Options &) {}
        };

        // ToString uses a different state.
        using FromStringState = DefaultState<FromStringOptions>;
        using ToBinaryState = DefaultState<ToBinaryOptions>;
        using FromBinaryState = DefaultState<FromBinaryOptions>;

        struct ToStringState : States::Base<ToStringState, ToStringOptions>
        {
          private:
            friend States;

            // Extra indentation at the beginning of each line, except the first one.
            int cur_indent = 0;

            void IncreaseNestingLevel(const ToStringOptions &options)
            {
                if (options.pretty)
                    cur_indent += options.indent;
            }

          public:
            int CurIndent() const
            {
                return cur_indent;
            }
        };

        struct UniversalInitialState
        {
            template <typename T, CHECK_EXPR(T::InitialState())>
            [[nodiscard]] operator T() const
            {
                return T::InitialState();
            }
        };
    }
    // Pass this as `state` parameter to `{To|From}{String|Binary}` interface functions.
    inline constexpr impl::UniversalInitialState initial_state;


    template <typename T>
    class InterfaceBasic : Meta::with_virtual_destructor<InterfaceBasic<T>>
    {
        static_assert(!std::is_reference_v<T>, "Can't make a reflection interface for a reference.");
        static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>, "Can't make a reflection interface for a type with cv-qualifiers.");

      public:
        constexpr InterfaceBasic() {}

        virtual void ToString(const T &object, Stream::Output &output, const ToStringOptions &options, impl::ToStringState state) const = 0;

        // This shouldn't skip any leading or trailing whitespace and comments, and shouldn't check for end of stream.
        virtual void FromString(T &object, Stream::Input &input, const FromStringOptions &options, impl::FromStringState state) const = 0;

        virtual void ToBinary(const T &object, Stream::Output &output, const ToBinaryOptions &options, impl::ToBinaryState state) const = 0;

        // This shouldn't check for end of stream.
        virtual void FromBinary(T &object, Stream::Input &input, const FromBinaryOptions &options, impl::FromBinaryState state) const = 0;
    };

    namespace impl
    {
        // Specializations should contain `using type = <interface>;`.
        template <typename T, typename = void>
        struct SelectInterface {};

        // Set this to `false` for scalars and similar types.
        // Do not set this to true for objects that have nested objects (unless you can guarantee
        // that all nested objects have this flag set too), otherwise conversion to string can yield weird results.
        template <typename T, typename = void>
        struct HasShortStringRepresentation : std::false_type {};
    }


    // Constructs an interface for a type.
    template <typename T>
    auto Interface() -> typename impl::SelectInterface<std::remove_cv_t<T>>::type
    {
        return {};
    }
    // A convenience overload for deducing T.
    // Note that this overload MUST NOT be called in an unqualified manner, to prevent unwanted ADL.
    template <typename T>
    auto Interface(const T &) -> typename impl::SelectInterface<std::remove_cv_t<T>>::type
    {
        return {};
    }


    inline namespace Shorthands
    {
        // Functions below use this wrapper for safery and convenience.
        // Otherwise e.g. `foo(ReadOnlydata)` could be called as `foo("bar")`, and would attempt to read `bar` as a file.
        class InputStreamWrapper
        {
            std::optional<Stream::Input> stream_storage;

          public:
            Stream::Input &stream;

            InputStreamWrapper(Stream::Input &stream) : stream(stream) {}
            InputStreamWrapper(Stream::Input &&stream) : stream(stream) {}

            InputStreamWrapper(std::string_view str)
                : stream_storage(std::in_place, Stream::ReadOnlyData::mem_reference(str)), stream(*stream_storage)
            {}

            InputStreamWrapper(const char *str) : InputStreamWrapper(std::string_view(str)) {}
            InputStreamWrapper(const std::string &str) : InputStreamWrapper(std::string_view(str)) {}

            // We use SFINAE to make sure this doesn't interfer with the copy/move ctors and with the string ctors.
            template <typename T, CHECK(!std::is_convertible_v<T, std::string_view> && !std::is_same_v<std::remove_cvref_t<T>, InputStreamWrapper>)>
            InputStreamWrapper(T &&param)
                : stream_storage(std::in_place, std::forward<T>(param)), stream(*stream_storage)
            {}

            // We need this to manually write this ctor to adjust the `stream` reference.
            // This prevents a move assignment from being generated, but we don't need it.
            InputStreamWrapper(InputStreamWrapper &&other) noexcept
                : stream_storage(std::move(other.stream_storage)), stream(stream_storage ? *stream_storage : other.stream)
            {}
        };


        template <typename T, CHECK_EXPR(Interface<T>())>
        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options = {})
        {
            Refl::Interface(object).ToString(object, output, options, initial_state); // A qualified call prevents unwanted ADL.
        }
        template <typename T, CHECK_EXPR(Interface<T>())>
        [[nodiscard]] std::string ToString(const T &object, const ToStringOptions &options = {})
        {
            std::string ret;
            Stream::Output output = Stream::Output::Container(ret);
            ToString(object, output, options);
            return ret;
        }


        // Skips any leading and trailing whitespace and comments. Expects `input` to have no junk at the end.
        template <typename T, CHECK_EXPR(Interface<T>())>
        void FromString(T &object, InputStreamWrapper input, const FromStringOptions &options = {})
        {
            input.stream.WantLocationStyle(Stream::text_position);
            Utils::SkipWhitespaceAndComments(input.stream);
            Refl::Interface(object).FromString(object, input.stream, options, initial_state); // A qualified call prevents unwanted ADL.
            Utils::SkipWhitespaceAndComments(input.stream);
            input.stream.ExpectEnd();
        }
        template <typename T, CHECK_EXPR(void(Interface<T>()), T{})>
        [[nodiscard]] T FromString(InputStreamWrapper input, const FromStringOptions &options = {})
        {
            T ret{};
            FromString(ret, std::move(input), options);
            return ret;
        }

        template <typename T, CHECK_EXPR(Interface<T>())>
        void ToBinary(const T &object, Stream::Output &output, const ToBinaryOptions &options = {})
        {
            Refl::Interface(object).ToBinary(object, output, options, initial_state); // A qualified call prevents unwanted ADL.
        }
        template <typename C, typename T, CHECK_EXPR(void(Interface<T>()), Stream::Output::Container(std::declval<C &>()))>
        [[nodiscard]] C ToBinary(const T &object, const ToBinaryOptions &options = {})
        {
            C ret;
            auto output = Stream::Output::Container(ret);
            ToBinary(object, output, options);
            output.Flush();
            return ret;
        }

        // Expects `input_data` to have no junk at the end.
        template <typename T, CHECK_EXPR(Interface<T>())>
        void FromBinary(T &object, InputStreamWrapper input, const FromBinaryOptions &options = {})
        {
            input.stream.WantLocationStyle(Stream::byte_offset);
            Refl::Interface(object).FromBinary(object, input.stream, options, initial_state); // A qualified call prevents unwanted ADL.
            input.stream.ExpectEnd();
        }
        template <typename T, CHECK_EXPR(void(Interface<T>()), T{})>
        [[nodiscard]] T FromBinary(InputStreamWrapper input, const FromBinaryOptions &options = {})
        {
            T ret{};
            FromBinary(ret, std::move(input), options);
            return ret;
        }
    }
}

/* Helper function to test serialization/deserialization:

    template <typename T> void TestRoundtripConversions(const T &object)
    {
        std::cout << "Testing roundtrip coversions for: " << Meta::TypeName<T>() << '\n';

        std::string as_str = Refl::ToString(object);
        std::string as_str_p = Refl::ToString(object, Refl::ToStringOptions::Pretty());
        auto as_vec = Refl::ToBinary<std::vector<unsigned char>>(object);

        std::cout << "As string: " << as_str << '\n';
        std::cout << "As pretty string: " << as_str_p << '\n';
        std::cout << "As vector: [";
        for (std::size_t i = 0; i < as_vec.size(); i++)
        {
            if (i != 0) std::cout << ',';
            std::cout << (int)as_vec[i];
        }
        std::cout << "]\n";

        auto Result = [&](const char *name, const T &result)
        {
            std::cout << name << ": " << (as_str == Refl::ToString(result) ? "OK" : "NOT OK <--------------------------") << '\n';
        };

        Result("String roundtrip ......", Refl::FromString<T>(as_str));
        Result("Pretty string roundtrip", Refl::FromString<T>(as_str_p));
        Result("Binary roundtrip ......", Refl::FromBinary<T>(Stream::ReadOnlyData::mem_reference(as_vec)));
    }
*/
