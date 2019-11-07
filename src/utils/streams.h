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
#include "utils/unicode.h"

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
        // Loads more data from the underlying data source.
        // Should return the actual amount of bytes read. If it returns 0, it's assumed that no more data is available.
        // The function object will never be copied, only moved. If you have a move-only lambda, you can use `Meta::fake_copyable`.
        using func_t = std::function<std::size_t(std::uint8_t *, std::size_t)>;

        enum class buffer_capacity_t : std::size_t {};
        static constexpr buffer_capacity_t default_buffer_capacity = buffer_capacity_t(512);

        struct Status
        {
            virtual std::string ToString() const = 0;
        };

        // Creates a user-facing status string, which normally contains a byte offset, or a line & column pair.
        // A stateful functor should be used. When it's called, the stored state should be updated.
        // The returned reference should probably point to the stored state. It will be used immediately after the call, or not used at all.
        // Can be null, then an empty string will be used.
        using status_func_t = std::function<const Status &(const uint8_t *, std::size_t)>;

        // Returns a byte offset.
        [[nodiscard]] static status_func_t Binary()
        {
            struct S : Status
            {
                std::size_t offset = 0;

                std::string ToString() const override
                {
                    char buffer[sizeof offset * 2 + 3]; // `offset * 2` means 2 hex digits per byte. 3 extra bytes are for the `0x` and the null-terminator.
                    std::snprintf(buffer, sizeof buffer, "%#zx", offset);
                    return buffer;
                }
            };

            return [state = S{}](const uint8_t *, std::size_t size) mutable -> const Status &
            {
                state.offset += size;
                return state;
            };
        }

        enum Encoding {utf8, ascii};

        // Returns a line and a column.
        [[nodiscard]] static status_func_t Text(Encoding enc = utf8)
        {
            struct S : Status
            {
                int line = 1;
                int column = 1;

                char prev_line_end = 0; // Used to handle different kinds of line endings.

                std::string ToString() const override
                {
                    return std::to_string(line) + ':' + std::to_string(column);
                }

                void AddCharacter(char ch)
                {
                    if (ch != '\n' && ch != '\r')
                    {
                        column++;
                        prev_line_end = 0;
                        return;
                    }

                    if (prev_line_end != 0 && ch != prev_line_end)
                    {
                        prev_line_end = 0;
                        return; // Skip the second byte of a line ending.
                    }

                    prev_line_end = ch;

                    column = 1;
                    line++;
                }
            };

            switch (enc)
            {
              case ascii:
                return [state = S{}](const uint8_t *ptr, std::size_t size) mutable -> const Status &
                {
                    for (std::size_t i = 0; i < size; i++)
                        state.AddCharacter(ptr[i]);
                    return state;
                };

              case utf8:
                return [state = S{}](const uint8_t *ptr, std::size_t size) mutable -> const Status &
                {
                    for (std::size_t i = 0; i < size; i++)
                    {
                        char ch = ptr[i];
                        if (Unicode::IsFirstByte(ch))
                            state.AddCharacter(ch);
                    }
                    return state;
                };
            }

            return 0;
        }

      private:
        struct Data
        {
            std::unique_ptr<std::uint8_t[]> buffer_storage; // The storage that `buffer` may or may not point to.
            std::uint8_t *buffer = 0; // The pointer to a buffer of size `buffer_capacity`.

            std::size_t buffer_capacity = 0; // How large the `buffer` is.

            std::size_t buffer_pos = 0; // The offset info `buffer`.
            std::size_t buffer_size = 0; // How much meaningful bytes the `buffer` has.
            // Normally `buffer_pos <= buffer_size`, except before the first read `buffer_pos == 1` and `buffer_size == 0`.

            // Returns the actual amount of bytes read. Throws on failure.
            // Should return 0 only on EOF.
            func_t obtain_bytes = 0;

            // Updates and returns a user-facing status string.
            // Can be null, then the string is assumed to be empty.
            status_func_t update_status = 0;
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
                if (data.update_status)
                    data.update_status(data.buffer, data.buffer_size);

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
                    throw std::runtime_error(GetExceptionPrefix() + "Unexpected end of an input stream.");
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
        Input(func_t obtain_bytes, status_func_t update_status = 0, buffer_capacity_t buffer_capacity = default_buffer_capacity)
        {
            data.buffer_storage = std::make_unique<std::uint8_t[]>(std::size_t(buffer_capacity));
            data.buffer = data.buffer_storage.get();
            data.buffer_capacity = std::size_t(buffer_capacity);
            data.buffer_pos = 1; // Sic.
            data.obtain_bytes = std::move(obtain_bytes);
            data.update_status = std::move(update_status);
        }

        // Constructs a stream referring to memory.
        Input(std::size_t size, const std::uint8_t *ptr, status_func_t update_status = 0)
        {
            data.buffer = const_cast<std::uint8_t *>(ptr); // Since we use a no-op `obtain_bytes`, nothing should ever write to this buffer.
            data.buffer_capacity = size;
            data.buffer_size = size;
            data.obtain_bytes = [](std::uint8_t *, std::size_t) {return std::size_t(0);};
            data.update_status = std::move(update_status);
        }

        // Constructs a stream referring to memory.
        // An alternative overload, with `char *` instead of `uint8_t *`.
        Input(std::size_t size, const char *ptr, status_func_t update_status = 0)
            : Input(size, reinterpret_cast<const std::uint8_t *>(ptr), std::move(update_status))
        {}

        // Constructs a stream referring to a file.
        Input(std::string file_name, status_func_t update_status = 0, buffer_capacity_t buffer_capacity = default_buffer_capacity)
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
            }), std::move(update_status), buffer_capacity);
        }

        // Constructs a stream referring to a file.
        // An alternative overload, using a C-string.
        Input(const char *file_name, status_func_t update_status = 0, buffer_capacity_t buffer_capacity = default_buffer_capacity)
            : Input(std::string(file_name), std::move(update_status), buffer_capacity)
        {}


        // Checks if the stream is null.
        [[nodiscard]] explicit operator bool() const {return bool(data.obtain_bytes);}

        // Checks if the stream wasn't accessed after the construction.
        bool WasNotAccessed() const
        {
            return data.buffer_pos > data.buffer_size;
        }

        // Updates the status function.
        // Does nothing if the function is already set, or if `Read*()` was called at least once on this stream.
        void SetStatusFunc(status_func_t update_status)
        {
            if (WasNotAccessed() && !data.update_status)
                data.update_status = std::move(update_status);
        }

        // Returns a user-friendly status string. Normally it's a byte offset, or a line and a column.
        // The call can be costly, so you should cache the result.
        std::string GetStatusString() const
        {
            if (!data.update_status)
                return "";

            auto update_status_copy = data.update_status; // We make a copy to avoid changing the state of the original.
            return update_status_copy(data.buffer, data.buffer_size).ToString();
        }

        // Returns a prefix that should be appended to exception messages.
        // Based on `GetStatusString()`.
        std::string GetExceptionPrefix() const
        {
            std::string status = GetStatusString();
            if (status.empty())
                return "";
            return "At " + status + ": ";
        }


        // Returns `false` if there is no more data.
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
                throw std::runtime_error(GetExceptionPrefix() + "Unexpected data at the end of an input stream.");
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
