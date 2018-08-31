#ifndef UTILS_RESOURCE_ALLOCATOR_H_INCLUDED
#define UTILS_RESOURCE_ALLOCATOR_H_INCLUDED

#include <cstddef>
#include <numeric>
#include <type_traits>
#include <utility>

#include "program/errors.h"

template <typename Res = int, typename Index = int> class ResourceAllocator
{
    static_assert(std::is_integral_v<Res> && std::is_integral_v<Index>, "Only integral types are supported.");

    Index pos = 0;
    std::vector<Res> resources;
    std::vector<Index> indices;

    using ResIterator = typename decltype(resources)::const_iterator;

  public:
    inline static constexpr Res none = -1;

    ResourceAllocator(Index size = 0)
    {
        Resize(size);
    }

    void Resize(Index size) // Frees all resources.
    {
        if (size < 0 || size == none) // `size == none` is for unsigned types.
            Program::Error("Invalid resource allocator size.");

        // We use temporary objects for strong exception guarantee.
        std::vector<Res> new_resources(size);
        std::vector<Index> new_indices(size);

        pos = 0;
        resources = std::move(new_resources);
        indices = std::move(new_indices);

        std::iota(resources.begin(), resources.end(), Res(0));
        std::iota(indices.begin(), indices.end(), Index(0));
    }

    Index Size() const
    {
        return resources.size();
    }
    Index ObjectsAllocated() const
    {
        return pos;
    }

    Res Alloc() // Returns `none` on failure.
    {
        if (pos >= Size())
            return none;
        return resources[pos++];
    }
    bool Free(Res res) // Returns 0 if the object wasn't allocated before.
    {
        if (res < 0 || res >= Size())
            return 0;
        pos--;
        Res last_res = resources[pos];
        std::swap(resources[indices[res]], resources[pos]);
        std::swap(indices[res], indices[last_res]);
        return 1;
    }
    void FreeEverything()
    {
        pos = 0; // It's ф completely safe thing to do.
    }

    ResIterator BeginAll() const
    {
        return resources.begin();
    }
    ResIterator EndAll() const
    {
        return resources.end();
    }
    ResIterator BeginAllocated() const
    {
        return resources.begin();
    }
    ResIterator EndAllocated() const
    {
        return resources.begin() + pos;
    }
    ResIterator BeginFree() const
    {
        return resources.begin() + pos;
    }
    ResIterator EndFree() const
    {
        return resources.end();
    }
};

#endif
