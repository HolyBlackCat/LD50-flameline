#pragma once

#include <cstddef>
#include <numeric>
#include <ostream>
#include <type_traits>
#include <utility>
#include <vector>

#include "macros/finally.h"
#include "meta/common.h"
#include "program/errors.h"

// Can store unique numbers, with values less than its capacity.
// The capacity can only be increased manually, and can never be decreased (without losing all elements).
// Like a vector, provides O(1) insertion and element access, O(n) erase (or O(1) if you don't care about preserving order).
// Also lets you find element indices in O(1), or check if they are present or not.
// Under the hood, uses two vectors of the specified capacity.
template <typename T = int>
class SparseSet
{
    static_assert(std::is_integral_v<T>, "The template parameter must be integral.");
  public:
    using elem_t = T;

  private:
    Meta::ResetIfMovedFrom<elem_t> pos = 0;

    // Those vectors have the same size.
    // Both contain unique sequental integers starting from 0.
    // At any point, `values[indices[x]] == x`, and vice versa.
    // `values` is always ordered so that the existing elements come first.
    std::vector<elem_t> values, indices;

  public:
    constexpr SparseSet() {}

    SparseSet(elem_t new_capacity)
    {
        Reserve(new_capacity);
    }

    // The maximum number of elements.
    [[nodiscard]] elem_t Capacity() const
    {
        return values.size();
    }
    // The current number of elements.
    [[nodiscard]] elem_t ElemCount() const
    {
        return pos.value;
    }
    // The amount of elements that can be inserted before the capacity is exhausted.
    [[nodiscard]] elem_t RemainingCapacity() const
    {
        return Capacity() - ElemCount();
    }
    // Returns true if the capacity is completely exhausted.
    [[nodiscard]] bool IsFull() const
    {
        return RemainingCapacity() == 0;
    }

    // Increase the capacity up to the specified value. Can't decrease capacity.
    void Reserve(elem_t new_capacity)
    {
        if (new_capacity <= Capacity())
            return;

        std::size_t old_capacity = Capacity();

        values.resize(new_capacity);
        FINALLY_ON_THROW( values.resize(old_capacity); )

        indices.resize(new_capacity);
        // Not needed since nothing throws below this point.
        // FINALLY_ON_THROW( indices.resize(old_capacity); )

        std::iota(values.begin() + old_capacity, values.end(), old_capacity);
        std::iota(indices.begin() + old_capacity, indices.end(), old_capacity);
    }

    // Returns true if the element exists in the set.
    // If the index is out of range, returns false instead of throwing.
    [[nodiscard]] bool Contains(elem_t elem) const
    {
        if (elem < 0 || elem >= Capacity())
            return false;
        return indices[elem] < ElemCount();
    }

    // Adds a new element to the set, a one that wasn't there before.
    // Throws if no free capacity.
    [[nodiscard]] elem_t InsertAny()
    {
        if (IsFull())
            throw std::runtime_error("Attempt to insert into a full `SparseSet`.");

        return values[pos.value++];
    }

    // Adds a new element to the set, returns true on success.
    // Returns false if the element was already present.
    bool Insert(elem_t elem)
    {
        if (Contains(elem))
            return false;

        elem_t last_index = pos.value++;
        elem_t this_index = indices[elem];
        elem_t last_value = values[last_index];
        elem_t this_value = elem;

        std::swap(values[this_index], values[last_index]);
        std::swap(indices[this_value], indices[last_value]);
        return true;
    }

    // Erases an element from the set, returns true on success.
    // Returns false if no such element.
    // Might change the element order.
    bool EraseUnordered(elem_t elem)
    {
        if (!Contains(elem))
            return false;

        elem_t last_index = --pos.value;
        elem_t this_index = indices[elem];
        elem_t last_value = values[last_index];
        elem_t this_value = elem;

        std::swap(values[this_index], values[last_index]);
        std::swap(indices[this_value], indices[last_value]);
        return true;
    }

    // Erases an element from the set, returns true on success.
    // Returns false if no such element.
    // Preserves the element order.
    bool EraseOrdered(elem_t elem)
    {
        if (!Contains(elem))
            return false;

        // Move elements.
        elem_t index = GetElemIndex(elem);
        std::rotate(values.begin() + index, values.begin() + index + 1, values.begin() + ElemCount());

        // Fix indices.
        // Note that we loop over the last element too.
        for (elem_t i = index; i < ElemCount(); i++)
            indices[values[i]] = i;

        // Decrement size.
        pos.value--;
        return true;
    }

    // Erases all elements while maintaining capacity.
    void EraseAllElements()
    {
        pos.value = 0;
    }

    // Returns i-th element.
    // If `index >= ElemCount()`, starts returning all missing elements.
    // If `index >= Capacity()` or is negative, throws.
    [[nodiscard]] elem_t GetElem(elem_t index) const
    {
        if (index < 0 || index >= Capacity())
            Program::Error("Out of range index for an `SparseSet` element.");
        return values[index];
    }

    // Returns the index of `elem` that can be used with `GetElem()`.
    // If the elem doesn't exist, returns `>= ElemCount()`.
    // If `elem >= Capacity()` or is negative, throws.
    elem_t GetElemIndex(elem_t elem) const
    {
        if (elem < 0 || elem >= Capacity())
            Program::Error("Out of range elem for an `SparseSet` index search.");
        return indices[elem];
    }

    // Prints the set and asserts consistency.
    template <typename ...P>
    void DebugPrint(std::basic_ostream<P...> &s)
    {
        // Print.
        s << '[';
        for (elem_t i = 0; i < ElemCount(); i++)
        {
            if (i != 0) s << ',';
            s << GetElem(i);
        }
        s << "]\n";

        // Assert consistency.
        ASSERT([&]{
            for (elem_t i = 0; i < Capacity(); i++)
            {
                if (GetElem(GetElemIndex(i)) != i)
                    return false;
            }
            return true;
        }(), "Consistency check failed for an `SparseSet`.");
    }
};
