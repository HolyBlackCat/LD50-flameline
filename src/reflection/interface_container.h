#pragma once

#include <cstddef>
#include <exception>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "meta/misc.h"
#include "program/errors.h"
#include "reflection/interface_basic.h"
#include "utils/robust_math.h"

namespace Refl
{
    namespace impl
    {
        // Container element type.
        // Specialize this for containers.
        // `T` will never be cv-qualified or a reference.
        template <typename T, typename = void> struct ContainerElem {/* using type = ...; */};

        // Changes container element type to make it mutable.
        // Normally you don't need to specialize this.
        template <typename T, typename = void> struct MakeMutable {using type = T;};
        template <typename T> struct MakeMutable<const T> {using type = T;};

        template <typename A, typename B> struct MakeMutable<std::pair<A,B>>
        {
            using type = std::pair<typename MakeMutable<A>::type, typename MakeMutable<B>::type>;
        };

        template <typename ...P> struct MakeMutable<std::tuple<P...>>
        {
            using type = std::tuple<typename MakeMutable<P>::type...>;
        };

        // If true, `T` won't be considered a container even if it looks like a container.
        template <typename T, typename = void> struct ForceNotContainer : std::false_type {};

        // These are used when converting containers to/from binary.
        using container_length_binary_t = std::uint32_t;
        inline constexpr auto container_length_byte_order = ByteOrder::little;
    }

    template <typename T>
    class Interface_BasicContainer : public InterfaceBasic<T>
    {
      public:
        // Element type.
        using elem_t = typename impl::ContainerElem<T>::type;

        // Mutable element type. Should be implicitly convertible to `elem_t`.
        using mutable_elem_t = typename impl::MakeMutable<elem_t>::type;

        [[nodiscard]] virtual std::size_t Size(const T &object) const = 0;

        // Does nothing if the container has no notion of capacity.
        virtual void Reserve(T &object, std::size_t capacity) const = 0;

        // Clears the container. Should set its capacity to zero if applicable.
        virtual void Clear(T &object) const = 0;

        // Inserts an element to the end of a container.
        // If the container doesn't allow duplicate elements and this one is a duplicate, should do nothing.
        virtual void PushBack(T &object, elem_t &&elem) const = 0;

        // Iterates over the container.
        virtual void ForEach(const T &object, std::function<void(const elem_t &elem)> func) const = 0;


        void ToString(const T &object, Stream::Output &output, const ToStringOptions &options, impl::ToStringState state) const override
        {
            constexpr bool force_single_line = impl::HasShortStringRepresentation<elem_t>::value;

            output.WriteChar('[');

            auto next_state = state.MemberOrElem(options);

            std::size_t index = 0, size = Size(object);
            ForEach(object, [&](const elem_t &elem)
            {
                if (options.pretty && !force_single_line)
                    output.WriteChar('\n').WriteChar(' ', state.CurIndent() + options.indent);

                Interface<mutable_elem_t>().ToString(elem, output, options, next_state);

                if (index != size-1 || (options.pretty && !force_single_line))
                {
                    output.WriteChar(',');
                    if (options.pretty && force_single_line)
                        output.WriteChar(' ');
                }

                index++;
            });

            if (options.pretty && !force_single_line && size > 0)
                output.WriteChar('\n').WriteChar(' ', state.CurIndent());

            output.WriteChar(']');
        }

        void FromString(T &object, Stream::Input &input, const FromStringOptions &options, impl::FromStringState state) const override
        {
            Clear(object);

            input.Discard('[');

            auto next_state = state.MemberOrElem(options);

            while (true)
            {
                Utils::SkipWhitespaceAndComments(input);
                if (input.Discard<Stream::if_present>(']'))
                    break;

                mutable_elem_t elem{};
                Interface<mutable_elem_t>().FromString(elem, input, options, next_state);

                try
                {
                    PushBack(object, std::move(elem));
                }
                catch (std::exception &e)
                {
                    Program::Error(input.GetExceptionPrefix() + e.what());
                }

                Utils::SkipWhitespaceAndComments(input);

                if (!input.Discard<Stream::if_present>(','))
                {
                    input.Discard(']');
                    break;
                }
            }
        }

        void ToBinary(const T &object, Stream::Output &output, const ToBinaryOptions &options, impl::ToBinaryState state) const override
        {
            impl::container_length_binary_t len;
            if (Robust::conversion_fails(object.size(), len))
                Program::Error(output.GetExceptionPrefix() + "The container is too long.");
            output.WriteWithByteOrder<impl::container_length_binary_t>(impl::container_length_byte_order, len);

            auto next_state = state.MemberOrElem(options);

            ForEach(object, [&](const elem_t &elem)
            {
                Interface<mutable_elem_t>().ToBinary(elem, output, options, next_state);
            });
        }

        void FromBinary(T &object, Stream::Input &input, const FromBinaryOptions &options, impl::FromBinaryState state) const override
        {
            std::size_t len;
            if (Robust::conversion_fails(input.ReadWithByteOrder<impl::container_length_binary_t>(impl::container_length_byte_order), len))
                Program::Error(input.GetExceptionPrefix() + "The string is too long.");

            std::size_t max_reserved_elems = options.max_reserved_size / sizeof(elem_t);

            Clear(object);
            Reserve(object, len < max_reserved_elems ? len : max_reserved_elems);

            auto next_state = state.MemberOrElem(options);

            while (len-- > 0)
            {
                mutable_elem_t elem{};
                Interface<mutable_elem_t>().FromBinary(elem, input, options, next_state);

                try
                {
                    PushBack(object, std::move(elem));
                }
                catch (std::exception &e)
                {
                    Program::Error(input.GetExceptionPrefix() + e.what());
                }
            }
        }
    };

    namespace impl::StdContainer
    {
        template <typename T> using has_reserve =
            decltype(std::declval<T &>().reserve(std::size_t{}));

        template <typename T> using has_push_back =
            decltype(std::declval<T &>().push_back(std::declval<const typename ContainerElem<T>::type &>()));

        template <typename T> using has_single_arg_insert =
            decltype(std::declval<T &>().insert(std::declval<const typename ContainerElem<T>::type &>()));

        // The type that `T::begin()` returns, stripped of any references and cv-qualifiers.
        template <typename T> using iter_t = std::decay_t<decltype(std::declval<const T &>().begin())>;

        // Checks for sane begin()/end().
        template <typename T> using has_sane_begin_end = decltype(
            void(++std::declval<iter_t<T> &>()),
            void(*std::declval<iter_t<T> &>()),
            void(std::declval<iter_t<T> &>() != std::declval<const T &>().end()),
            // Make sure `*begin()` returns a reference.
            std::enable_if_t<std::is_reference_v<decltype(*std::declval<T &>().begin())>>()
        );

        // Check if a type looks like a container.
        // It has to have sane `begin()` and `end()`, and either `push_back` or single-arg `insert` (so only variable-length arrays are allowed).
        template <typename T> inline constexpr bool is_container =
            Meta::is_detected<has_sane_begin_end, T> && (Meta::is_detected<has_push_back, T> || Meta::is_detected<has_single_arg_insert, T>) && !ForceNotContainer<T>::value;
    }

    template <typename T>
    class Interface_StdContainer final : public Interface_BasicContainer<T>
    {
        static constexpr bool has_push_back = Meta::is_detected<impl::StdContainer::has_push_back, T>;

      public:
        using typename Interface_BasicContainer<T>::elem_t;

        [[nodiscard]] virtual std::size_t Size(const T &object) const override
        {
            return object.size();
        }

        virtual void Reserve(T &object, std::size_t capacity) const override
        {
            if constexpr (Meta::is_detected<impl::StdContainer::has_reserve, T>)
                object.reserve(capacity);
        }

        virtual void Clear(T &object) const override
        {
            // Note that `= {}` is not good enough, since it appears to
            // invoke the `initializer_list`, which requires elements to be copyable.
            object = T{};
        }

        virtual void PushBack(T &object, elem_t &&elem) const override
        {
            if constexpr (has_push_back)
                object.push_back(std::move(elem));
            else
                object.insert(std::move(elem));
        }

        virtual void ForEach(const T &object, std::function<void(const elem_t &elem)> func) const override
        {
            for (auto it = object.begin(); it != object.end(); it++)
                func(*it);
        }
    };

    template <typename T>
    struct impl::SelectInterface<T, std::enable_if_t<impl::StdContainer::is_container<T>>>
    {
        using type = Interface_StdContainer<T>;
    };

    template <typename T>
    struct impl::ContainerElem<T, std::enable_if_t<Meta::is_detected<impl::StdContainer::has_sane_begin_end, T>>> // Note that we can't use `impl::StdContainer::is_container<T>` here, since it relies on `ContainerElem`.
    {
        using type = std::remove_reference_t<decltype(*std::declval<T &>().begin())>;
    };
}
