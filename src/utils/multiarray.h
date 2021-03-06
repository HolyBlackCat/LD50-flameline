#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <type_traits>
#include <vector>
#include <utility>

#include "meta/lists.h"
#include "program/errors.h"
#include "strings/format.h"
#include "utils/mat.h"


template <int D, typename T>
class MultiArray
{
  public:
    static constexpr int dimensions = D;
    static_assert(dimensions >= 2, "Arrays with less than 2 dimensions are not supported.");
    static_assert(dimensions <= 4, "Arrays with more than 4 dimensions are not supported.");

    using type = T;
    using index_t = std::ptrdiff_t;
    using index_vec_t = index_vec<D>;

    struct ReflHelper; // Our reflection metadata uses this to access private fields.

  private:
    index_vec_t size_vec{};
    std::vector<type> storage;

  public:
    constexpr MultiArray() {}

    MultiArray(index_vec_t size_vec) : size_vec(size_vec), storage(size_vec.prod())
    {
        ASSERT(size_vec.min() >= 0, "Invalid multiarray size.");
    }
    MultiArray(index_vec_t size_vec, const T &init) : size_vec(size_vec), storage(size_vec.prod(), init)
    {
        ASSERT(size_vec.min() >= 0, "Invalid multiarray size.");
    }
    template <typename A, A ...I>
    MultiArray(Meta::value_list<I...>, std::array<type, index_vec_t(I...).prod()> data) : size_vec(I...), storage(data.begin(), data.end())
    {
        static_assert(std::is_integral_v<A>, "Indices must be integral.");
        static_assert(((I >= 0) && ...), "Invalid multiarray size.");
    }

    [[nodiscard]] index_vec_t size() const
    {
        return size_vec;
    }

    [[nodiscard]] bool pos_in_range(index_vec_t pos) const
    {
        return (pos >= 0).all() && (pos < size_vec).all();
    }

    [[nodiscard]] type &unsafe_at(index_vec_t pos)
    {
        ASSERT(pos_in_range(pos), STR("Multiarray indices out of range. Indices are ", (pos), " but the array size is ", (size_vec), "."));

        index_t index = 0;
        index_t factor = 1;

        for (int i = 0; i < dimensions; i++)
        {
            index += factor * pos[i];
            factor *= size_vec[i];
        }

        return storage[index];
    }
    [[nodiscard]] type &safe_throwing_at(index_vec_t pos)
    {
        if (!pos_in_range(pos))
            Program::Error("Multiarray index ", pos, " is out of range. The array size is ", size_vec, ".");
        return unsafe_at(pos);
    }
    [[nodiscard]] type &safe_nonthrowing_at(index_vec_t pos)
    {
        if (!pos_in_range(pos))
            Program::HardError("Multiarray index ", pos, " is out of range. The array size is ", size_vec, ".");
        return unsafe_at(pos);
    }
    [[nodiscard]] type &clamped_at(index_vec_t pos)
    {
        clamp_var(pos, 0, size_vec-1);
        return unsafe_at(pos);
    }
    [[nodiscard]] type try_get(index_vec_t pos)
    {
        if (!pos_in_range(pos))
            return {};
        return unsafe_at(pos);
    }
    void try_set(index_vec_t pos, const type &obj)
    {
        if (!pos_in_range(pos))
            return;
        unsafe_at(pos) = obj;
    }
    void try_set(index_vec_t pos, type &&obj)
    {
        if (!pos_in_range(pos))
            return;
        unsafe_at(pos) = std::move(obj);
    }

    [[nodiscard]] const type &unsafe_at(index_vec_t pos) const
    {
        return const_cast<MultiArray *>(this)->unsafe_at(pos);
    }
    [[nodiscard]] const type &safe_throwing_at(index_vec_t pos) const
    {
        return const_cast<MultiArray *>(this)->safe_throwing_at(pos);
    }
    [[nodiscard]] const type &safe_nonthrowing_at(index_vec_t pos) const
    {
        return const_cast<MultiArray *>(this)->safe_nonthrowing_at(pos);
    }
    [[nodiscard]] const type &clamped_at(index_vec_t pos) const
    {
        return const_cast<MultiArray *>(this)->clamped_at(pos);
    }
    [[nodiscard]] type try_get(index_vec_t pos) const
    {
        return const_cast<MultiArray *>(this)->try_get(pos);
    }
    void try_set(index_vec_t pos, const type &obj) const
    {
        return const_cast<MultiArray *>(this)->try_set(pos, obj);
    }
    void try_set(index_vec_t pos, type &&obj) const
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

template <typename T> using Array2D = MultiArray<2, T>;
template <typename T> using Array3D = MultiArray<3, T>;
template <typename T> using Array4D = MultiArray<4, T>;
