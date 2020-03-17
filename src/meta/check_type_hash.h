#pragma once

#include <cstddef>
#include <type_traits>

#include "meta/misc.h"
#include "meta/type_info.h"
#include "program/errors.h"

#ifndef NDEBUG
#define TYPE_HASH_CHECKS_ENABLED
#endif

namespace Meta
{
    namespace impl
    {
        // A class wrapper to make friend declarations easier.
        struct TypeId
        {
            ~TypeId() = default;

            // Affects class hashes.
            // If you feel like you got a hash collision (invalid casts are not being detected), try changing this value.
            static constexpr Meta::hash_t hash_seed = 123;

            class Support
            {
                constexpr Support(Meta::hash_t hash) : hash(hash) {}

                volatile Meta::hash_t hash;
              public:
                const volatile Meta::hash_t &Get() const {return hash;}

                // cvref-qualifiers on T are ignored.
                template <typename T>
                [[nodiscard]] static Support Make()
                {
                    return Support(Meta::TypeHash<std::remove_cvref_t<T>>(hash_seed));
                }

                Support(const Support &other) : hash(other.hash) {}
                Support &operator=(const Support &) {return *this;} // This does nothing intentionally.

                ~Support()
                {
                    hash = 0;
                }
            };

            #ifdef TYPE_HASH_CHECKS_ENABLED
            // Can't use `Meta::is_detected` because the variable is not necessarily public.
            // Note that we not only check the validity of the member pointer, but also its type.
            // This ensures that inherited class members don't count.
            template <typename T, typename = void> struct stores_hash : std::false_type {};
            template <typename T> struct stores_hash<T,
                std::enable_if_t<std::is_same_v<Support T::*, decltype(&T::_type_hash_storage_)>>> : std::true_type {};
            #endif

            template <typename T> [[nodiscard]] static constexpr bool CheckHash(const T &object)
            {
                #ifdef TYPE_HASH_CHECKS_ENABLED
                static_assert(std::is_class_v<T>, "The parameter must have a class type.");
                static_assert(impl::TypeId::stores_hash<T>::value, "This type doesn't store its hash. Add `STORE_TYPE_HASH` to the class body.");
                return object._type_hash_storage_.Get() == TypeHash<T>(hash_seed);
                #else
                (void)object;
                return true;
                #endif
            }
        };
    }

    // In release builds does nothing (if `TYPE_HASH_CHECKS_ENABLED` is not defined).
    // Otherwise, triggers an assertion if `object` doesn't refer to an actual object of type `T`.
    // If it's not possible to check this object (class lacks `STORE_TYPE_HASH` in its body), a `static_assert` is triggered.
    template <typename T>
    void AssertTypeHash(const T &object)
    {
        (void)object;
        DebugAssert("Type hash check failed.", impl::TypeId::CheckHash(object));
    }
}

#ifdef TYPE_HASH_CHECKS_ENABLED
// Add this to a class definition to enable member downcast support for it.
// In debug builds this increases class size by 4.
#define STORE_TYPE_HASH \
    friend ::Meta::impl::TypeId; \
    ::Meta::impl::TypeId::Support _type_hash_storage_ = ::Meta::impl::TypeId::Support::Make<decltype(*this)>();
#else
#define STORE_TYPE_HASH
#endif
