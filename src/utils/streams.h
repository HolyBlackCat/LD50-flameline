#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "utils/byte_order.h"
#include "utils/memory_access.h"
#include "utils/meta.h"

/* Simple IO streams, which can be bound to files, memory or some other storagee.
 *
 * The input stream, when tested on a bunch of one-byte reads, appears to be faster
 *   than a plain `FILE *` in release mode (more than 50% faster), but slower in debug mode (almost 50% slower).
 *   As read size increases, the speed difference decreases. On four-byte reads, streams
 *   are 25% faster in release mode, and almost 33% slower in debug mode.
 */

namespace Stream
{
    class Input : Meta::with_virtual_destructor<Input>
    {
      public:
        using func_t = std::function<std::size_t(std::uint8_t *, std::size_t)>;
        enum class buffer_capacity_t : std::size_t {};

        static constexpr buffer_capacity_t default_buffer_capacity = buffer_capacity_t(512); // The same buffer capacity appears to be used by `<cstdio>` by default.

      private:
        struct Data
        {
            std::unique_ptr<std::uint8_t[]> buffer_storage; // The storage that `buffer` may or may not point to.
            std::uint8_t *buffer = 0; // The pointer to a buffer of size `buffer_capacity`.

            std::size_t buffer_capacity = 0; // How large the `buffer` is.

            std::size_t buffer_pos = 0; // The offset info `buffer`.
            std::size_t buffer_size = 0; // How much meaningful bytes the `buffer` has.
            // Normally `buffer_pos <= buffer_size`, except before the first read `buffer_pos == 1` and `buffer_size == 0` (on a non-default-construted stream).

            // Returns the actual amount of bytes read. Throws on failure.
            // Should return 0 only on EOF.
            func_t obtain_bytes = 0;
        };
        Data data;

        // Returns `true` if the buffer doesn't have any more useful data in it, but it can be obtained with `FillBufferWithNewData()`.
        // Returns `false` on EOF.
        bool BufferIsEmpty()
        {
            return data.buffer_pos >= data.buffer_size; // We use `>=` rather than `==`, since before the first read `buffer_pos(1) > buffer_size(0)`.
        }

        // Uses `obtain_bytes` to replace contents of `buffer` with new data.
        void FillBufferWithNewData()
        {
            assert(BufferIsEmpty());

            if (data.buffer_pos > 0) // Don't attempt reading if EOF is reached.
            {
                data.buffer_pos = 0;
                data.buffer_size = 0; // Note that we assign a zero first. If `obtain_bytes` throws, this will cause the stream to end up in the EOF state.
                data.buffer_size = data.obtain_bytes(data.buffer, data.buffer_capacity);
            }
        }

        // If the buffer is empty, attempts to fill it and throws on failure.
        void NeedData()
        {
            if (BufferIsEmpty())
            {
                FillBufferWithNewData();

                if (BufferIsEmpty())
                    throw std::runtime_error("Unexpected end of an input stream.");
            }
        }

      public:
        Input() {} // Constructs an empty stream.

        Input(Input &&other) noexcept : data(std::exchange(other.data, {})) {}
        Input &operator=(Input other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        // Constructs a stream referring to an arbitrary data source.
        Input(func_t obtain_bytes, buffer_capacity_t buffer_capacity = default_buffer_capacity)
        {
            data.buffer_storage = std::make_unique<std::uint8_t[]>(std::size_t(buffer_capacity));
            data.buffer = data.buffer_storage.get();
            data.buffer_capacity = std::size_t(buffer_capacity);
            data.buffer_pos = 1; // Sic.
            data.obtain_bytes = std::move(obtain_bytes);
        }

        // Constructs a stream referring to memory.
        Input(std::size_t size, const std::uint8_t *ptr)
        {
            data.buffer = const_cast<std::uint8_t *>(ptr); // Since we use a no-op `obtain_bytes`, nothing should ever write to this buffer.
            data.buffer_capacity = size;
            data.buffer_size = size;
            data.obtain_bytes = [](std::uint8_t *, std::size_t) {return std::size_t(0);};
        }

        // Constructs a stream referring to memory.
        // An alternative overload, with `char *` instead of `uint8_t *`.
        Input(std::size_t size, const char *ptr)
            : Input(size, reinterpret_cast<const std::uint8_t *>(ptr))
        {}

        // Constructs a stream referring to a file.
        Input(std::string file_name, buffer_capacity_t buffer_capacity = default_buffer_capacity)
        {
            auto deleter = [](FILE *file){std::fclose(file);}; // `fclose` can fail, but it destroys `FILE *` even in that case, so we don't check the return value.
            std::unique_ptr<FILE, decltype(deleter)> handle(std::fopen(file_name.c_str(), "rb"));
            if (!handle)
                throw std::runtime_error("Unable to open `" + file_name + "` for reading.");

            // Disable file buffering, since we use a custom buffer.
            // This doesn't indicate a error in any way, but if it did, we would ignore it.
            setbuf(handle.get(), nullptr);

            *this = Input(Meta::fake_copyable([handle = std::move(handle), file_name = std::move(file_name)](std::uint8_t *dest, std::size_t size) -> std::size_t
            {
                std::size_t result = std::fread(dest, 1, size, handle.get());
                if (result < size)
                {
                    if (std::ferror(handle.get()))
                        throw std::runtime_error("Unable to read from `" + file_name + "`.");
                }

                return result;
            }), buffer_capacity);
        }

        // Constructs a stream referring to a file.
        // An alternative overload, using a C-string.
        Input(const char *file_name, buffer_capacity_t buffer_capacity = default_buffer_capacity)
            : Input(std::string(file_name), buffer_capacity)
        {}


        // Checks if the stream is null.
        [[nodiscard]] explicit operator bool() const {return bool(data.obtain_bytes);}

        // Returns `false` if there is no more data (and for default-constructed streams).
        // It's non-const because it might attempt to fill the buffer with new data.
        [[nodiscard]] bool HasMoreData()
        {
            if (BufferIsEmpty())
                FillBufferWithNewData();

            return data.buffer_pos < data.buffer_size;
        }

        // Throws if the stream has more data.
        void ExpectEnd()
        {
            if (HasMoreData())
                throw std::runtime_error("Unexpected data at the end of an input stream.");
        }

        // Peeks the next byte in the stream, throws on failure.
        [[nodiscard]] std::uint8_t PeekByte()
        {
            NeedData();
            return data.buffer[data.buffer_pos];
        }

        // Attempts to read a single byte, throws on failure.
        [[nodiscard]] std::uint8_t ReadByte()
        {
            NeedData();
            return data.buffer[data.buffer_pos++];
        }

        // Attempts to read several bytes, throws on failure.
        // `dest` can be null, then the bytes are discarded.
        void ReadBytes(std::uint8_t *dest, std::size_t count)
        {
            while (count > 0)
            {
                NeedData();
                std::size_t segment_size = std::min(count, data.buffer_size - data.buffer_pos);
                if (dest)
                    std::memcpy(dest, data.buffer + data.buffer_pos, segment_size);
                data.buffer_pos += segment_size;
                count -= segment_size;
                dest += segment_size;
            }
        }

        // Attempts to discard several bytes, throws on failure.
        void DiscardBytes(std::size_t count)
        {
            ReadBytes(0, count);
        }

        // Reads an arithmetic type with a specified byte order.
        template <typename T>
        [[nodiscard]] T ReadWithOrder(ByteOrder::Order order)
        {
            T ret;
            std::uint8_t *ptr = reinterpret_cast<std::uint8_t *>(&ret);
            ReadBytes(ptr, sizeof ret);
            ByteOrder::ConvertBytes(ptr, sizeof ret, order);
            return ret;
        }

        // Reads an arithmetic type with the native byte order.
        template <typename T>
        [[nodiscard]] T ReadNative()
        {
            return ReadWithOrder<T>(ByteOrder::native);
        }

        // Reads an arithmetic type with the little-endian byte order.
        template <typename T>
        [[nodiscard]] T ReadLittle()
        {
            return ReadWithOrder<T>(ByteOrder::little);
        }

        // Reads an arithmetic type with the big-endian byte order.
        template <typename T>
        [[nodiscard]] T ReadBig()
        {
            return ReadWithOrder<T>(ByteOrder::big);
        }

        // Reads a null-terminated string.
        [[nodiscard]] std::string ReadNullTerminatedString()
        {
            std::string ret;
            while (char ch = ReadByte())
                ret += ch;
            return ret;
        }
    };
}
