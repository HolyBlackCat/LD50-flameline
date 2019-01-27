#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <type_traits>
#include <vector>
#include <utility>

#include "program/errors.h"
#include "utils/mat.h"
#include "utils/meta.h"

template <int D, typename T> class MultiArray
{
  public:
    static constexpr int dimensions = D;
    static_assert(dimensions > 0, "Attempt to create multiarray with 0 dimensions.");
    static_assert(dimensions <= 4, "Multiarrays with more than 4 dimensions are not supported.");

    using type = T;
    using index_t = std::ptrdiff_t;
    using extents_t = std::conditional_t<dimensions == 1, index_t, vec<D, index_t>>;

  private:
    extents_t extent_vec;
    std::vector<type> storage;

  public:
    MultiArray(extents_t extent_vec = extents_t(0)) : extent_vec(extent_vec), storage(extent_vec.prod())
    {
        DebugAssert("Invalid multiarray size.", extent_vec.min() >= 0);
    }
    template <index_t ...I> MultiArray(Meta::value_list<I...>, std::array<type, extents_t(I...).prod()> data) : extent_vec(I...), storage(data.begin(), data.end())
    {
        static_assert(((I >= 0) && ...), "Invalid multiarray size.");
    }

    [[nodiscard]] extents_t extents() const
    {
        return extent_vec;
    }

    [[nodiscard]] bool pos_in_range(extents_t pos) const
    {
        return (pos >= 0).all() && (pos < extent_vec).all();
    }

    [[nodiscard]] type &unsafe_at(extents_t pos)
    {
        DebugAssert(Str("Multiarray indices out of range. Indices are ", pos, " but the array size is ", extent_vec, "."), pos_in_range(pos));

        index_t index = 0;
        index_t factor = 1;

        for (int i = 0; i < dimensions; i++)
        {
            index += factor * pos[i];
            factor *= extent_vec[i];
        }

        return storage[index];
    }
    [[nodiscard]] type &clamp_at(extents_t pos)
    {
        clamp_var(pos, 0, extent_vec-1);
        return unsafe_at(pos);
    }
    [[nodiscard]] type try_get(extents_t pos)
    {
        if (!pos_in_range(pos))
            return {};
        return unsafe_at(pos);
    }
    void try_set(extents_t pos, const type &obj)
    {
        if (!pos_in_range(pos))
            return;
        return unsafe_at(pos) = obj;
    }
    void try_set(extents_t pos, type &&obj)
    {
        if (!pos_in_range(pos))
            return;
        return unsafe_at(pos) = std::move(obj);
    }

    [[nodiscard]] const type &unsafe_at(extents_t pos) const
    {
        return const_cast<MultiArray *>(this)->unsafe_at(pos);
    }
    [[nodiscard]] const type &clamp_at(extents_t pos) const
    {
        return const_cast<MultiArray *>(this)->clamp_at(pos);
    }
    [[nodiscard]] type try_get(extents_t pos) const
    {
        return const_cast<MultiArray *>(this)->try_get(pos);
    }
    void try_set(extents_t pos, const type &obj) const
    {
        return const_cast<MultiArray *>(this)->try_set(pos, obj);
    }
    void try_set(extents_t pos, type &&obj) const
    {
        return const_cast<MultiArray *>(this)->try_set(pos, std::move(obj));
    }

    [[nodiscard]] index_t element_count() const
    {
        return storage.size();
    }
    [[nodiscard]] type *elements()
    {
        return storage.data();
    }
    [[nodiscard]] const type *elements() const
    {
        return storage.data();
    }
};
