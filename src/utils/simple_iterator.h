#pragma once

#include <cstddef>
#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>

#include "macros/check.h"
#include "meta/assignable_wrapper.h"

namespace SimpleIterator
{
    namespace impl
    {
        template <typename T, typename = void> struct is_eq_comparable : std::false_type {};
        template <typename T> struct is_eq_comparable<T, decltype(std::declval<const T &>() == std::declval<const T &>(), void())> : std::true_type {};

        enum class Mode {input, forward};

        template <Mode IterMode, typename State>
        class Iter
        {
            static constexpr bool state_is_eq_comparable = is_eq_comparable<State>::value;

            static_assert(std::is_invocable_v<State, std::false_type> && std::is_invocable_v<State &, std::true_type>, "State must be callable as `state(std::true|false_type{})`.");
            static_assert(IterMode != Mode::forward || std::is_default_constructible_v<State>, "Forward iterators must have default-constructible state.");
            static_assert(std::is_copy_constructible_v<State>, "Iterators must have copy-constructible state.");
            // No assertion for assignable-ness, because we have a wrapper that can make the state assignable.
            static_assert(IterMode != Mode::forward || state_is_eq_comparable, "State of forward iterators must be equality-comparable.");

            using wrapped_state_t = std::conditional_t<std::is_copy_assignable_v<State>, State, Meta::AssignableWrapper<State>>;

            // The state is `mutable` because we invoke `operator()` from `operator*`, which is `const`.
            // We could require `operator()` to be `const` instead, but it's not very convenient, especially for lambdas.
            [[no_unique_address]] mutable wrapped_state_t state;

            using callback_ret_t = std::invoke_result_t<State &, std::false_type>;
            static_assert(std::is_lvalue_reference_v<callback_ret_t> || (IterMode == Mode::input && !std::is_reference_v<callback_ret_t>),
                          "`state(std::false_type{})` must return an lvalue reference (possibly const); or for an input iterator, possibly return by value.");

          public:
            using value_type = std::remove_cvref_t<callback_ret_t>;
            using difference_type = std::ptrdiff_t;
            using reference = callback_ret_t;
            using pointer = std::remove_reference_t<callback_ret_t> *;
            using iterator_category = std::conditional_t<IterMode == Mode::input, std::input_iterator_tag, std::forward_iterator_tag>;

            // Doesn't have to compile for input iterators.
            Iter() : state() {}

            Iter(const State &state) : state(state) {}
            Iter(State &&state) : state(std::move(state)) {}

            // `Iter` is copyable, which is a part of iterator requirements.

            reference operator*() const
            {
                return state(std::false_type{});
            }
            pointer operator->() const
            {
                // I don't want to include `<memory>` for `std::addressof`.
                return reinterpret_cast<pointer>(&reinterpret_cast<const char &>(operator*()));
            }

            Iter &operator++()
            {
                (void)state(std::true_type{});
                return *this;
            }
            Iter operator++(int)
            {
                Iter ret(*this);
                operator++();
                return ret;
            }

            bool operator==(const Iter &other) const
            {
                if constexpr (state_is_eq_comparable)
                {
                    // If the state is equality-comparable, use `==`.
                    return state == other.state;
                }
                else
                {
                    // If the state is not equality-comparable, compare the addresses.
                    // Input iterators must be equality-comparable, but nothing is said about the quality of their `operator==`.
                    // Forward iterators, on the other hand, require a proper `operator==`, so we ensure their state has it with a `static_assert`.
                    return this == &other;
                }
            }
            bool operator!=(const Iter &other) const
            {
                return !operator==(other);
            }
        };
    }


    // An input iterator.
    // The state must be callable as `state(std::true_type{})` (which should increment the iterator),
    // and as `state(std::false_type{})` (which should either return an lvalue reference to the current element, possibly const; or return the element by value).
    // If the reference returned by `state(std::false_type{})` is non-const, then it's possible to assign to a dereferenced iterator.
    // When invoked, the state will always be considered non-const (it's stored as `mutable`).
    // State must be copy constructible. It should also be copy-assignable; if not, copy assignment will be implemented as in-place reconstruction. Same for move constructor and assignment.
    // If state overloads `==`, then then `==` of the iterator will use it. Otherwise `==` will compare object addresses, because we're required to provide it.
    // We're allowed to do it, because requirements for `==` for input iterators are somewhat relaxed.
    template <typename State>
    struct Input : impl::Iter<impl::Mode::input, State>
    {
        using impl::Iter<impl::Mode::input, State>::Iter;
    };
    template <typename State> Input(const State &) -> Input<State>;
    template <typename State> Input(State &&) -> Input<State>;

    // A forward iterator.
    // Imposes the same requirements on `State` as `SimpleIterator::Input, with some exceptions:
    // `State` must be default-constructible.
    // `State` must overload `==`.
    template <typename State>
    struct Forward : impl::Iter<impl::Mode::forward, State>
    {
        using impl::Iter<impl::Mode::forward, State>::Iter;
    };
    template <typename State> Forward(const State &) -> Forward<State>;
    template <typename State> Forward(State &&) -> Forward<State>;


    // Equivalent to `Input(func)`.
    // This function is there so that you don't have to think about which iterator type to pick.
    template <typename F>
    [[nodiscard]] auto Make(F &&func)
    {
        return Input(std::forward<F>(func));
    }

    // Constructs a forward iterator with the value type matching the type of first parameter (`remove_cvref_t<S>`).
    // `func` is `void func(remove_cvref_t<S> &)`, it's used to modify the value when the iterator is incremented.
    // Types of both `state` and `func` don't have to be default-constructible (default-constructed iterators are
    // non-dereferencable and non-incrementable).
    // `==` is overloaded for the resulting iterator to compare the stored values (and default-constructed iterators
    // compare equal to each other, and inequal to everything else).
    template <typename V, typename F>
    [[nodiscard]] auto Make(V &&value, F &&func)
    {
        struct State
        {
            struct Data
            {
                std::remove_cvref_t<V> value;
                std::remove_cvref_t<F> func;

                Data(V &&value, F &&func) : value(std::forward<V>(value)), func(std::forward<F>(func)) {}

                bool operator==(const Data &other) const
                {
                    return value == other.value;
                }
            };
            std::optional<Data> data;

            const auto &operator()(std::false_type) const
            {
                return data->value;
            }
            void operator()(std::true_type)
            {
                data->func(data->value);
            }

            bool operator==(const State &other) const
            {
                return data == other.data;
            }
        };
        return Forward<State>(State{typename State::Data(std::forward<V>(value), std::forward<F>(func))});
    }
}
