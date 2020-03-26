#pragma once

#if __has_include("utils/mat.h")

#include <tuple>

#include "reflection/structs_basic.h"
#include "utils/mat.h"

namespace Refl::Class::Custom
{
    template <std::size_t D, typename M> struct name<Math::vec<D, M>>
    {
        static constexpr const char *value = "vec";
    };
    template <std::size_t D, typename M> struct members<Math::vec<D, M>>
    {
        using T = Math::vec<D, M>;
        static constexpr std::size_t count = D;
        template <std::size_t I> static constexpr auto &at(T &object)
        {
            return object.template get<I>();
        }
    };

    template <std::size_t W, std::size_t H, typename M> struct name<Math::mat<W, H, M>>
    {
        static constexpr const char *value = "mat";
    };
    template <std::size_t W, std::size_t H, typename M> struct members<Math::mat<W, H, M>>
    {
        using T = Math::mat<W, H, M>;
        static constexpr std::size_t count = W * H;
        template <std::size_t I> static constexpr auto &at(T &object)
        {
            Math::vec<H, M> *v = nullptr;
                 if constexpr (I % W == 0) v = &object.x;
            else if constexpr (I % W == 1) v = &object.y;
            else if constexpr (I % W == 2) v = &object.z;
            else                           v = &object.w;
            return v->template get<I / W>();
        }
    };
}

#endif
