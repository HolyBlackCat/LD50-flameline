#pragma once

/* Member downcasts

This file contains macros to "downcast" pointers/references to class members to pointers/references to enclosing classes.

Example usage:

    struct A
    {
        int x = 42;
        float y = 12.3;
        STORE_TYPE_HASH // Can be anywhere in the class.
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
by an `offsetof` (and does a few `static_assert`s). STORE_TYPE_HASH expands to nothing.

In debug builds however, `STORE_TYPE_HASH` expands to a member variable containing the enclosing class type hash.
This variable is checked by the casts, so invalid casts are detected at runtime.

Note that the second parameter (the member name) can accept a `.`-separated chain of members.
*/

#include "meta/check_type_hash.h"

namespace Macro::MemberDowncast
{
    template <typename OuterT, typename InnerT, std::ptrdiff_t MemberOffset, typename ParamT>
    [[nodiscard]] static auto *Ptr(ParamT *ptr)
    {
        static_assert(std::is_class_v<OuterT> && !std::is_const_v<OuterT> && !std::is_volatile_v<OuterT>, "Resulting type must be a cv-unqualified class.");
        static_assert(std::is_same_v<std::remove_cvref_t<InnerT>, std::remove_cvref_t<ParamT>>, "Argument type doesn't match the type of the specified member.");

        using outer_type_cv_t = Meta::copy_qualifiers<ParamT, OuterT>;
        using char_cv_t = Meta::copy_qualifiers<ParamT, char>;

        auto *ret = reinterpret_cast<outer_type_cv_t *>(reinterpret_cast<char_cv_t *>(ptr) - MemberOffset);
        Meta::AssertTypeHash(*ret);

        return ret;
    }

    template <typename OuterT, typename InnerT, std::ptrdiff_t MemberOffset, typename ParamT>
    [[nodiscard]] static auto &Ref(ParamT &ref)
    {
        return *Ptr<OuterT, InnerT, MemberOffset>(&ref);
    }
}

#define MEMBER_DOWNCAST_PTR(_class, _member_name, ...) ::Macro::MemberDowncast::Ptr<_class, decltype(_class::_member_name), offsetof(_class, _member_name)>(__VA_ARGS__)
#define MEMBER_DOWNCAST(_class, _member_name, ...) ::Macro::MemberDowncast::Ref<_class, decltype(_class::_member_name), offsetof(_class, _member_name)>(__VA_ARGS__)
