#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>

#include "utils/byte_order.h"

namespace Memory
{
    template <typename T> [[nodiscard]] T Read(const uint8_t *ptr, ByteOrder::Order order = ByteOrder::native)
    {
        T object;
        std::memcpy(&object, ptr, sizeof(T));
        ByteOrder::Convert(object, order);
        return object;
    }
    template <typename T> [[nodiscard]] T ReadLittle(const uint8_t *ptr)
    {
        return Read<T>(ptr, ByteOrder::little);
    }
    template <typename T> [[nodiscard]] T ReadBig(const uint8_t *ptr)
    {
        return Read<T>(ptr, ByteOrder::big);
    }

    template <typename T> void Write(uint8_t *ptr, const std::enable_if_t<1, T> &object, ByteOrder::Order order = ByteOrder::native)
    {
        std::memcpy(ptr, &object, sizeof(T));
        ByteOrder::ConvertBytes(ptr, sizeof(T), order);
    }
    template <typename T> void WriteLittle(const uint8_t *ptr, const std::enable_if_t<1, T> &object)
    {
        Write<T>(ptr, object, ByteOrder::little);
    }
    template <typename T> void WriteBig(const uint8_t *ptr, const std::enable_if_t<1, T> &object)
    {
        Write<T>(ptr, object, ByteOrder::big);
    }
}
