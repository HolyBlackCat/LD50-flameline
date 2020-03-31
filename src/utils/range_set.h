#pragma once

#include <algorithm>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <vector>

#include "utils/simple_iterator.h"

template <typename T>
class RangeSet
{
    static_assert(std::is_integral_v<T>, "The template parameter must be integral.");

  public:
    using elem_type = T;

    struct Range
    {
        T begin = 1;
        T end = 0; // The end is inclusive!

        constexpr Range() {}
        constexpr Range(T value) : begin(value), end(value) {}

        [[nodiscard]] constexpr static Range OffsetAndSize(T begin, T size)
        {
            return HalfOpen(begin, begin + size);
        }
        [[nodiscard]] constexpr static Range HalfOpen(T begin, T end)
        {
            return Inclusive(begin, end - 1);
        }
        [[nodiscard]] constexpr static Range Inclusive(T begin, T end)
        {
            Range ret;
            ret.begin = begin;
            ret.end = end;
            return ret;
        }

        // Returns true if the range is valid and non-empty.
        [[nodiscard]] constexpr explicit operator bool() const
        {
            return begin <= end;
        }

        [[nodiscard]] constexpr bool Contains(T value) const
        {
            return value >= begin && value <= end;
        }
    };

  private:
    mutable std::vector<Range> ranges; // None of those ranges can be empty.
    mutable bool dirty = false;

    struct IteratorState
    {
        typename std::vector<Range>::const_iterator range_iter{}, range_iter_end{};
        elem_type value{};

        IteratorState() {}
        IteratorState(const std::vector<Range> &vec) : range_iter(vec.begin()), range_iter_end(vec.end())
        {
            if (!Finished())
                value = range_iter->begin;
        }

        bool Finished() const
        {
            return range_iter == range_iter_end;
        }

        const T &operator()(std::false_type) const
        {
            return value;
        }
        void operator()(std::true_type)
        {
            bool last_value = value >= range_iter->end; // Can't compare after incrementing, because it might overflow.

            value++;

            if (last_value)
            {
                range_iter++;
                if (!Finished())
                    value = range_iter->begin;
            }
        }

        bool operator==(const IteratorState &other) const
        {
            bool finished = Finished();
            bool other_finished = other.Finished();
            if (finished && other_finished)
                return true;
            if (finished != other_finished)
                return false;
            return range_iter == other.range_iter && value == other.value;
        }
    };

  public:
    RangeSet() {}

    RangeSet(const Range &range) {Add(range);}
    RangeSet(std::vector<Range> ranges) : ranges(std::move(ranges)) {}

    RangeSet &Add(Range range)
    {
        if (!range)
            return *this;

        ranges.push_back(range);
        dirty = true;
        return *this;
    }

    // Sort ranges and merge overlapping ones.
    // Normally you don't need to call this function, this is done automatically.
    const RangeSet &Normalize() const
    {
        if (!dirty || ranges.empty()) // The code below assumes `ranges.size() > 0`.
            return *this;

        // Sort ranges by `begin`.
        std::sort(ranges.begin(), ranges.end(), [](const Range &a, const Range &b)
        {
            return a.begin < b.begin;
        });

        std::size_t new_size = 1; // Note that we start from 1.
        for (std::size_t i = 1; i < ranges.size(); i++) // Also start from 1 here.
        {
            auto &last_range = ranges[new_size-1];
            auto &this_range = ranges[i];

            if (last_range.end + 1 >= this_range.begin)
            {
                if (last_range.end < this_range.end)
                    last_range.end = this_range.end;
            }
            else
            {
                ranges[new_size++] = ranges[i];
            }
        }
        ranges.resize(new_size);

        return *this;
    }

    [[nodiscard]] auto begin() const
    {
        Normalize();
        return SimpleIterator::Forward<IteratorState>(ranges);
    }
    [[nodiscard]] auto end() const
    {
        // No need to normalize here.
        return SimpleIterator::Forward<IteratorState>();
    }

    // Returns the list of ranges, sorted and without overlapping.
    [[nodiscard]] const std::vector<Range> &Ranges() const
    {
        Normalize();
        return ranges;
    }

    [[nodiscard]] bool Contains(T value) const
    {
        Normalize();
        auto range_iter = std::lower_bound(ranges.begin(), ranges.end(), value, [](const Range &range, T value){return range.end < value;});
        return range_iter != ranges.end() && range_iter->Contains(value);
    }
};
