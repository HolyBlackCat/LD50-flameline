#pragma once

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <utility>

#include "meta/misc.h"

namespace Hash
{
    inline void Append(std::size_t &dst, std::size_t src)
    {
        dst ^= src + std::size_t(0x9E3779B97F4A7C16) + (dst << 6) + (dst >> 2); // That number is a fractional part of the golden ratio. Boost uses a similar thing.
    }

    inline void Append(std::size_t &dst, std::initializer_list<std::size_t> src)
    {
        for (std::size_t it : src)
            Append(dst, it);
    }

    [[nodiscard]] inline std::size_t Combine(std::size_t a, std::size_t b)
    {
        Append(a, b);
        return a;
    }

    [[nodiscard]] inline std::size_t Combine(std::initializer_list<std::size_t> list)
    {
        auto it = list.begin();
        if (it == list.end())
            return 0;
        std::size_t hash = *it++;
        while (it != list.end())
            Append(hash, *it++);
        return hash;
    }


    namespace Custom
    {
        inline std::size_t hash(/* T &object */) = delete; // Overload this to provide custom hashes. You can also use ADL.
    }


    namespace impl
    {
        template <typename TT> using supports_custom = decltype(hash(std::declval<const TT &>()));
        template <typename TT> using supports_std    = decltype(std::declval<const std::hash<TT> &>()(std::declval<const TT &>()));
        template <typename TT> using supports_member = decltype(std::declval<const TT &>().hash());
    }

    template <typename T> [[nodiscard]] std::size_t Compute(const T &obj)
    {
        if constexpr (Meta::is_detected<impl::supports_custom, T>)
        {
            using Custom::hash; // This enables ADL it it's needed.
            return hash(obj);
        }
        else if constexpr (Meta::is_detected<impl::supports_std, T>)
        {
            return std::hash<T>{}(obj);
        }
        else if constexpr (Meta::is_detected<impl::supports_member, T>)
        {
            return obj.hash();
        }
        else
        {
            static_assert(Meta::value<false, T>, "This type can't be hashed.");
            return 0;
        }
    }

    template <typename ...P> [[nodiscard]] std::size_t Compute(const P &... params)
    {
        return Combine({Compute(params)...});
    }

    struct Obj
    {
        template <typename T> [[nodiscard]] std::size_t operator()(const T &obj) const
        {
            return Compute(obj);
        }
    };


    // Custom hashes.
    namespace Custom
    {
        // Pairs.
        template <typename A, typename B> std::size_t hash(const std::pair<A, B> &pair)
        {
            return Compute(pair.first, pair.second);
        }

        // Tuples.
        template <typename ...P> std::size_t hash(const std::tuple<P...> &tuple)
        {
            return std::apply(Compute<P...>, tuple);
        }
    }
}
