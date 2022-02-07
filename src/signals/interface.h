#pragma once

#include <concepts>
#include <type_traits>

#include "meta/common.h"

namespace Sig
{
    namespace impl
    {
        struct InterfaceHelper
        {
            InterfaceHelper() = default;
            InterfaceHelper(const InterfaceHelper &) = delete;
            InterfaceHelper &operator=(const InterfaceHelper &) = delete;
            ~InterfaceHelper() = default;

            template <typename T> requires std::is_same_v<typename std::remove_reference_t<T>::interface_helper, InterfaceHelper>
            using InterfaceType = Meta::copy_cvref_qualifiers<T, typename std::remove_reference_t<T>::interface_type>;

            template <typename T>
            [[nodiscard]] InterfaceType<T &&> operator()(T &&object) const
            {
                return static_cast<InterfaceType<T &&>>(std::forward<T>(object));
            }
        };
    }

    // Exposes an interface of an object by casting it to one of its private bases.
    // For a class to support this function, it has to provide following:
    //     friend impl::InterfaceHelper;
    //     using interface_helper = impl::InterfaceHelper;
    //     using interface_type = PrivateInterfaceBase;
    // It's also recommended to declare the class itself as `[[nodiscard]]`, since lambdas can't use this attribute.
    // This is a lambda rather than a function to prevent unwanted ADL.
    constexpr impl::InterfaceHelper Interface; // auto &&Interface(auto &&object);

    // The return type of `Interface()`.
    // Cvref-qualifiers of `T` are preserved.
    template <typename T>
    using InterfaceType = impl::InterfaceHelper::InterfaceType<T>;
}
