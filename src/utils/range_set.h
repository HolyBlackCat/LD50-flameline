#pragma once

#include <map>
#include <type_traits>
#include "utils/mat.h"

template <typename T> class RangeSet
{
    static_assert(std::is_integral_v<T>, "The template parameter must be integral.");

    std::map<T, T> map; // This map countains non-overlapping inclusive ranges.
    T values = 0;

    using iterator_t = typename decltype(map)::iterator;

    void SetEndOfRange(iterator_t it, const T &new_end)
    {
        values += new_end - it->second; // This should work even with unsigned wraparound.
        it->second = new_end;
    }
    void SetEndOfRangeToMax(iterator_t it, const T &new_end)
    {
        if (new_end > it->second)
            SetEndOfRange(it, new_end);
    }
    void EraseRange(iterator_t it)
    {
        values -= it->second - it->first + 1;
        map.erase(it);
    }

    void TryMergingAdjacent(iterator_t a, iterator_t &b) // Makes `b` equal to `a` if the ranges had to be merged. Note that only `b` is passed by reference.
    {
        if (a->second >= b->first-1)
        {
            SetEndOfRangeToMax(a, b->second);
            EraseRange(b);
            b = a;
        }
    }

  public:
    RangeSet() {}

    void Add(const T &first, const T &last) // The range is inclusive.
    {
        // Try inserting.
        auto [it_cur, inserted] = map.insert({first, last});

        if (inserted)
        {
            // Update value count.
            values += last - first + 1;

            // Check for intersections with previous range.
            if (it_cur != map.begin())
                TryMergingAdjacent(std::prev(it_cur), it_cur);
        }
        else
        {
            // Update end of the existing range.
            SetEndOfRangeToMax(it_cur, last);
        }

        // Check for intersections with next range.
        if (auto it_next = std::next(it_cur); it_next != map.end())
            TryMergingAdjacent(it_cur, it_next);
    }

    void Add(const T &value)
    {
        Add(value, value);
    }

    void Clear()
    {
        map = {};
        values = 0;
    }

    int RangeCount() const
    {
        return map.size();
    }
    T ValueCount() const
    {
        return values;
    }

    template <typename F> void ForEachRange(F &&func/* void(const T &begin, const T &end) */) const // Both ends of ranges are inclusive.
    {
        for (auto [begin, end] : map)
            func(begin, end);
    }
    template <typename F> void ForEachValue(F &&func/* void(const T &value) */) const
    {
        for (auto [begin, end] : map)
        for (auto it = begin; it <= end; it++)
            func(it);
    }
};
