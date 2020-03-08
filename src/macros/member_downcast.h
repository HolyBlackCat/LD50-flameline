#pragma once

/* Member downcasts

This file contains macros to "downcast" pointers/references to class members to pointers/references to enclosing classes.

Example usage:

    struct A
    {
        int x = 42;
        float y = 12.3;
        MEMBER_DOWNCAST_SUPPORT // Can be anywhere in the class.
    };

    int main()
    {
        A a;
        auto &ax = a.x;
        auto &ay = a.y;
        std::cout << MEMBER_DOWNCAST(A, x, ax).y << '\n'; // 12.3
        std::cout << MEMBER_DOWNCAST_PTR(A, y, &ay)->x << '\n'; // 42

        return 0;
    }

In release builds, `MEMBER_DOWNCAST[_PTR]` merely shifts the pointer
by an `offsetof` (and does a few `static_assert`s). MEMBER_DOWNCAST_SUPPORT expands to nothing.

In debug builds however, `MEMBER_DOWNCAST_SUPPORT` expands to a member variable containing enclosing class type hash.
This variable is checked by the casts, so invalid casts are detected at runtime.

*/

#include <cstddef>
#include <type_traits>

#include "meta/misc.h"
#include "meta/type_info.h"
#include "program/errors.h"

#ifndef NDEBUG
#define MEMBER_DOWNCAST_CHECKS_ENABLED
#endif

namespace Macro
{
    // A class wrapper to make friend declarations easier.
    struct MemberDowncast
    {
        ~MemberDowncast() = default;

        // Affects class hashes.
        // If you feel like you got a hash collision (invalid casts are not being detected), try changing this value.
        static constexpr Meta::hash_t class_hash_seed = 123;

        class Support
        {
            constexpr Support(Meta::hash_t hash) : hash(hash) {}

          public:
            const Meta::hash_t hash;

            // cvref-qualifiers on T are ignored.
            template <typename T>
            [[nodiscard]] static Support Make()
            {
                return Support(Meta::TypeHash<std::remove_cvref_t<T>>(class_hash_seed));
            }

            constexpr Support(const Support &other) : hash(other.hash) {}
            constexpr Support &operator=(const Support &) {return *this;} // This does nothing intentionally.
        };

        #ifdef MEMBER_DOWNCAST_CHECKS_ENABLED
        // Can't use `Meta::is_detected` because the variable is not necessarily public.
        template <typename T, typename = void> struct has_downcast_support : std::false_type {};
        template <typename T> struct has_downcast_support<T, decltype(void(&T::_member_downcast_support_))> : std::true_type {};
        #endif

        template <typename OuterT, typename InnerT, std::ptrdiff_t MemberOffset, typename ParamT>
        [[nodiscard]] static auto *Ptr(ParamT *ptr)
        {
            static_assert(std::is_class_v<OuterT> && !std::is_const_v<OuterT> && !std::is_volatile_v<OuterT>, "Resulting type must be a cv-unqualified class.");
            static_assert(std::is_same_v<std::remove_cvref_t<InnerT>, std::remove_cvref_t<ParamT>>, "Argument type doesn't match the type of the specified member.");
            #ifdef MEMBER_DOWNCAST_CHECKS_ENABLED
            static_assert(has_downcast_support<OuterT>::value, "Resulting type doesn't support member downcast, add MEMBER_DOWNCAST_SUPPORT.");
            #endif

            using outer_type_cv_t = Meta::copy_qualifiers<InnerT, OuterT>;
            using char_cv_t = Meta::copy_qualifiers<InnerT, char>;
            auto *ret = reinterpret_cast<outer_type_cv_t *>(reinterpret_cast<char_cv_t *>(ptr) - MemberOffset);

            #ifdef MEMBER_DOWNCAST_CHECKS_ENABLED
            DebugAssert("Invalid member downcast.", ret->_member_downcast_support_.hash == Meta::TypeHash<OuterT>(class_hash_seed));
            #endif

            return ret;
        }

        template <typename OuterT, typename InnerT, std::ptrdiff_t MemberOffset, typename ParamT>
        [[nodiscard]] static auto &Ref(ParamT &ref)
        {
            return *Ptr<OuterT, InnerT, MemberOffset>(&ref);
        }
    };
}

#ifdef MEMBER_DOWNCAST_CHECKS_ENABLED
// Add this to a class definition to enable member downcast support for it.
// In debug builds this increases class size by 4.
#define MEMBER_DOWNCAST_SUPPORT \
    friend ::Macro::MemberDowncast; \
    const ::Macro::MemberDowncast::Support _member_downcast_support_ = ::Macro::MemberDowncast::Support::Make<decltype(*this)>();
#else
#define MEMBER_DOWNCAST_SUPPORT
#endif

// foo
#define MEMBER_DOWNCAST_PTR(_class, _member_name, _member) ::Macro::MemberDowncast::Ptr<_class, decltype(_class::_member_name), offsetof(_class, _member_name)>(_member)
#define MEMBER_DOWNCAST(_class, _member_name, _member) ::Macro::MemberDowncast::Ref<_class, decltype(_class::_member_name), offsetof(_class, _member_name)>(_member)
