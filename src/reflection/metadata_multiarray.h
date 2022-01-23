#pragma once

#include <cstddef>

#include "program/errors.h"
#include "reflection/interface_struct.h"
#include "utils/multiarray.h"

template <int D, typename T> struct MultiArray<D, T>::ReflHelper
{
    static auto &GetSizeVec(MultiArray<D, T> &array)
    {
        return array.size_vec;
    }

    static auto &GetStorage(MultiArray<D, T> &array)
    {
        return array.storage;
    }

    static void CheckInvariant(const MultiArray<D, T> &object)
    {
        if ((object.size_vec < 0).any())
            Program::Error("Multiarray can't have a negative size.");

        if (std::size_t(object.size_vec.prod()) != object.storage.size())
            Program::Error("Multiarray size doesn't match the number of elements in the storage.");
    }
};

namespace Refl::Class::Custom
{
    template <int D, typename T> struct name<MultiArray<D, T>>
    {
        static constexpr const char *value = "MultiArray";
    };
    template <int D, typename T> struct members<MultiArray<D, T>>
    {
        static constexpr std::size_t count = 2;
        template <std::size_t I> static constexpr auto &at(MultiArray<D, T> &object)
        {
            if constexpr (I == 0)
                return MultiArray<D, T>::ReflHelper::GetSizeVec(object);
            else
                return MultiArray<D, T>::ReflHelper::GetStorage(object);
        }
    };
}

template <int D, typename T>
struct Refl::StructCallbacks<MultiArray<D, T>> : Refl::DefaultStructCallbacks<MultiArray<D, T>>
{
    static void PreSerialize(const MultiArray<D, T> &object)
    {
        MultiArray<D, T>::ReflHelper::CheckInvariant(object);
    }
    static void PostDeserialize(MultiArray<D, T> &object)
    {
        MultiArray<D, T>::ReflHelper::CheckInvariant(object);
    }
};
