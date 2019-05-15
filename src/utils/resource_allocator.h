#pragma once

#include <cstddef>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include "program/errors.h"

template <typename T = int>
class ResourceAllocator
{
  public:
    using type = T;

  private:
    static_assert(std::is_integral_v<type>, "The template parameter must be integral.");

    struct Data
    {
        type pos = 0;
        std::vector<type> values, indices;

        Data(type size = 0) : values(size), indices(size) {}
    };
    Data data;

    void ReserveMore()
    {
        // Make sure this handles `Capacity() == 0` correctly.
        Reserve(Capacity() * 3 / 2 + 1);
    }

  public:
    using ResIterator = typename decltype(data.values)::const_iterator;

    ResourceAllocator(type size = 0) : data(size)
    {
        std::iota(data.values.begin(), data.values.end(), type(0));
        std::iota(data.indices.begin(), data.indices.end(), type(0));
    }

    ResourceAllocator(const ResourceAllocator &other) : data(other.data) {}
    ResourceAllocator(ResourceAllocator &&other) noexcept : data(std::exchange(other.data, {})) {}
    ResourceAllocator &operator=(ResourceAllocator other) noexcept
    {
        std::swap(data, other.data);
        return *this;
    }

    type Capacity() const
    {
        return data.values.size();
    }
    type ObjectsAllocated() const
    {
        return data.pos;
    }
    type RemainingCapacity() const
    {
        return Capacity() - ObjectsAllocated();
    }

    void Reserve(type new_capacity) // This can only increase capacity.
    {
        if (new_capacity <= Capacity())
            return;

        ResourceAllocator new_object;

        auto lambda = [&](auto member_ptr)
        {
            auto &old_vec = data.*member_ptr;
            auto &new_vec = new_object.data.*member_ptr;

            new_vec.reserve(new_capacity);
            new_vec.insert(new_vec.end(), old_vec.begin(), old_vec.end());
            new_vec.insert(new_vec.end(), new_capacity - Capacity(), 0);

            std::iota(new_vec.begin() + Capacity(), new_vec.end(), Capacity());
        };

        new_object.data.pos = data.pos;
        lambda(&Data::values);
        lambda(&Data::indices);

        *this = std::move(new_object);
    }

    bool IsAllocated(type object) const
    {
        if (object < 0 || object >= Capacity())
            return 0;
        return data.indices[object] < ObjectsAllocated();
    }

    type Allocate() // Allocates an object. Increases capacity if necessary.
    {
        if (data.pos >= Capacity())
            ReserveMore();

        return data.values[data.pos++];
    }

    void AllocateObject(type object) // Attempts to allocate a specific object, throws on failure. Increases capacity if necessary.
    {
        if (IsAllocated(object))
            Program::Error("Attempt to allocate object `", object, "` that is already allocated.");

        Reserve(object + 1);

        type last_index = data.pos++;
        type this_index = data.indices[object];
        type last_value = data.values[last_index];
        type this_value = object;

        std::swap(data.values[this_index], data.values[last_index]);
        std::swap(data.indices[this_value], data.indices[last_value]);
    }

    void Free(type object) // Frees an object. Throws if `object` wasn't allocated.
    {
        if (!IsAllocated(object))
            Program::Error("Attempt to free object `", object, "` that wasn't allocated.");

        type last_index = --data.pos;
        type this_index = data.indices[object];
        type last_value = data.values[last_index];
        type this_value = object;

        std::swap(data.values[this_index], data.values[last_index]);
        std::swap(data.indices[this_value], data.indices[last_value]);
    }

    void FreeAllObjects() // Frees all objects while maintaining capacity.
    {
        data.pos = 0;
    }

    type GetAllocatedObject(type pos) const // pos < ObjectsAllocated()
    {
        if (pos < 0 || pos >= ObjectsAllocated())
            Program::Error("Allocated object index is out of range.");

        return data.indices[pos];
    }

    type GetFreeObject(type pos) const // pos < RemainingCapacity()
    {
        if (pos < 0 || pos >= RemainingCapacity()())
            Program::Error("Free object index is out of range.");

        return data.indices[ObjectsAllocated() + pos];
    }

    template <typename F> void ForEachAllocatedObject(F &&func) // `func` is `void func(type)`.
    {
        for (type i = 0; i < ObjectsAllocated(); i++)
            func(GetAllocatedObject(i));
    }

    template <typename F> void ForEachFreeObject(F &&func) // `func` is `void func(type)`.
    {
        for (type i = 0; i < RemainingCapacity(); i++)
            func(GetFreeObject(i));
    }
};
