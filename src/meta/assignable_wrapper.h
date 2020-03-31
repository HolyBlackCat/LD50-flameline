#pragma once

#include <exception> // For `std::terminate`.
#include <new> // For `std::launder`.
#include <type_traits>
#include <utility>

namespace Meta
{
    // Wraps an object of type `T`. If it's not copy/move assignable, attempts
    // to implement the corresponding assignment operators by reconstructing the object.
    // If the move constructor or assignment operator of `T` throws, `std::terminate` is called.
    // This is primarily useful for making lambdas assignable.

    template <typename T>
    union AssignableWrapper
    {
        // We're using a `union` to be able to manually control the lifetime of `value`.
        // Otherwise the destructor for it would be called without `std::launder`, which is probably not good.

        // Note the lack of `static_assert`s for `T` being copy/move-constructible.
        // It doesn't have to be, if we never instantiate the corresponding member functions.

        [[no_unique_address]] T value;

      public:
        AssignableWrapper() : value{} {}
        AssignableWrapper(const T &value) : value(value) {}
        AssignableWrapper(T &&value) : value(std::move(value)) {}

        // Obtains the wrapped value.
        T &Get()
        {
            // If `T` is not copy- or move-assignable, then it might've been reconstructed, and we need to launder it.
            if constexpr (std::is_copy_assignable_v<T> && std::is_move_assignable_v<T>)
                return value;
            else
                return *std::launder(&value);
        }
        const T &Get() const
        {
            return const_cast<AssignableWrapper &>(*this).Get();
        }

        AssignableWrapper(const AssignableWrapper &other) : value(other.value) {}
        AssignableWrapper(AssignableWrapper &&other) : value(std::move(other.value)) {}

        AssignableWrapper &operator=(const AssignableWrapper &other)
        {
            if constexpr (std::is_copy_assignable_v<T>)
            {
                // `T` is assignable, use its `operator=`.
                Get() = other.Get();
            }
            else if constexpr (std::is_nothrow_copy_constructible_v<T>)
            {
                // `T` is not assignable, but its nothrow copy constructible. Reconstruct it without worrying about exceptions.
                if (this == &other)
                    return *this;

                Get().T::~T(); // Using a qualified destructor call to avoid possible unnecessary virtual dispatch.
                new(&value) T(other.Get());
            }
            else
            {
                // `T` is not assignable, and its copy constructor can throw. Reconstruct it in a safe manner.
                if (this == &other)
                    return *this;

                // Move value into a temporary object, in case the copy constructor throws.
                T value_copy(std::move(Get()));

                // Destroy the old object.
                Get().T::~T(); // Using a qualified destructor call to avoid possible unnecessary virtual dispatch.

                try
                {
                    // Try constructing a new one.
                    new(&value) T(other.Get());
                }
                catch (...)
                {
                    try
                    {
                        // Can't construct the new value, try rolling back to the old one.
                        new(&value) T(std::move(value_copy));
                    }
                    catch (...)
                    {
                        // Can't roll back, abort.
                        std::terminate();
                    }

                    // Rolled back to the old value, rethrow the exception.
                    throw;
                }
            }

            return *this;
        }

        AssignableWrapper &operator=(AssignableWrapper &&other) noexcept // Note that unlike copy assignment, this is unconditionally noexcept.
        {
            if constexpr (std::is_move_assignable_v<T>)
            {
                // `T` is assignable, use its `operator=`.

                // Yes, `std::is_move_assignable_v<T>` is true even if `T` only has a copy assignment operator.
                // Not sure how to work around that, but it only matters for performance reasons, in a single case:
                // if `T` defines copy/move ctors but only a copy assignment. I don't think a sensible class would do that.
                Get() = std::move(other.Get());
            }
            else
            {
                // `T` is not assignable. Reconstruct it without worrying about exceptions, since we're in a noexcept function.
                if (this == &other)
                    return *this;

                Get().T::~T(); // Using a qualified destructor call to avoid possible unnecessary virtual dispatch.
                // No exception protection needed, because we're in a `noexcept` function.
                new(&value) T(std::move(other.Get()));
            }

            return *this;
        }

        ~AssignableWrapper()
        {
            // Manually destroy the object.
            Get().T::~T(); // Using a qualified destructor call to avoid possible unnecessary virtual dispatch.
        }

        // Forward the `()` operator to the value, why not.
        template <typename ...P> [[nodiscard]] decltype(auto) operator()(P &&... params)       {return Get()(std::forward<P>(params)...);}
        template <typename ...P> [[nodiscard]] decltype(auto) operator()(P &&... params) const {return Get()(std::forward<P>(params)...);}
    };
}
