#pragma once

#include <algorithm>
#include <cstring>
#include <vector>

#include "program/errors.h"
#include "reflection/full_with_poly.h"
#include "utils/poly_storage.h"

template <typename Base>
requires Refl::Class::explicitly_polymorphic<Base>
class ActionSequence
{
    using storage_t = Refl::PolyStorage<Base>;

    static const std::vector<storage_t> &GetList()
    {
        static std::vector<storage_t> ret = []{
            std::vector<storage_t> ret;
            std::size_t count = Refl::Polymorphic::DerivedClassCount<Base>();
            ret.reserve(count);
            for (std::size_t i = 0; i < count; i++)
                ret.push_back(Refl::Polymorphic::ConstructFromIndex<Base>(i));

            // Make sure the list is sorted alphabetically (by class names).
            // Reflection is supposed to do that automatically, but we add an assertion to be sure.
            auto compare = [](const storage_t &a, const storage_t &b){return std::strcmp(Refl::Polymorphic::Name(a), Refl::Polymorphic::Name(b)) < 0;};
            ASSERT(std::is_sorted(ret.begin(), ret.end(), compare));
            return ret;
        }();
        return ret;
    }

  public:
    constexpr ActionSequence() {}

    // Returns the amount of stored actions.
    [[nodiscard]] std::size_t size() const
    {
        return GetList().size();
    }

    // Those iterators dereference to `const Refl::PolyStorage<Base> &`.
    // The list of objects is stored in a global variable, and is constructed on the first use.
    // The `ActionSequence` object itself is empty, and can be constructed as needed.
    [[nodiscard]] auto begin() const
    {
        return GetList().begin();
    }
    [[nodiscard]] auto end() const
    {
        return GetList().end();
    }
};
