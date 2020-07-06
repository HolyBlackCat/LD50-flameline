#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include "macros/check.h"
#include "macros/finally.h"
#include "meta/misc.h"
#include "program/errors.h"
#include "stream/better_fopen.h"
#include "stream/readonly_data.h"
#include "stream/save_to_file.h"
#include "stream/utils.h"
#include "strings/format.h"
#include "utils/byte_order.h"
#include "utils/unicode.h"

namespace Stream
{
    class Output
    {
      public:
        static constexpr capacity_t default_capacity = capacity_t(512); // This is what `FILE *` appears to use by default.

        // Flushes bytes to the underlying object.
        // Can throw on failure.
        // Will never be copied. If your functor is non-copyable, consider using `Meta::fake_copyable`.
        using flush_func_t = std::function<void(Output &, const std::uint8_t *, std::size_t)>;

      private:
        struct Data
        {
            std::unique_ptr<std::uint8_t[]> buffer;
            std::size_t buffer_pos = 0;
            std::size_t buffer_capacity = 0;
            flush_func_t flush;

            std::optional<ExceptionPrefixStyle> exception_prefix_style;

            std::string name;
        };
        Data data;

        void NeedBufferSpace()
        {
            if (data.buffer_pos == data.buffer_capacity)
                Flush();
        }

      public:
        // Constructs an empty stream.
        Output() {}

        Output(Output &&other) noexcept : data(std::exchange(other.data, {})) {}
        Output &operator=(Output other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        ~Output() // If `Flush` throws, the destructor swallows the exception.
        {
            try
            {
                ASSERT(data.buffer_pos == 0, "Suggest flushing the stream before destroying it.\nNot doing so in a release build will silently ignore any possible exceptions.");

                Flush();
            }
            catch (...) {}
        }

        // Constructs a stream with an arbitrary underlying object.
        Output(std::string name, flush_func_t flush, capacity_t capacity = default_capacity)
        {
            data.buffer = std::make_unique<std::uint8_t[]>(std::size_t(capacity));
            data.buffer_capacity = std::size_t(capacity);
            data.flush = std::move(flush);
            data.name = std::move(name);
        }

        // Constructs a stream bound to a file.
        Output(std::string file_name, SaveMode mode = binary, capacity_t capacity = default_capacity)
        {
            auto deleter = [](FILE *file)
            {
                // We don't check for errors here, since there is nothing we could do.
                // And `file` is always closed anyway, even if `fclose` doesn't return `0`
                std::fclose(file);
            };

            std::unique_ptr<FILE, decltype(deleter)> handle(better_fopen(file_name.c_str(), SaveModeStringRepresentation(mode)));
            if (!handle)
                Program::Error("Unable to open `" + file_name + "` for writing.");

            // This function can fail, but it doesn't report errors in any way.
            // Even if it did, we would still ignore it.
            std::setbuf(handle.get(), nullptr);

            *this = Output(std::move(file_name),
                Meta::fake_copyable([handle = std::move(handle)](const Output &object, const std::uint8_t *data, std::size_t size)
                {
                    if (!std::fwrite(data, size, 1, handle.get()))
                        Program::Error(object.GetExceptionPrefix() + "Unable to write to file.");
                }),
                capacity);
        }

        // Constructs a stream bound to a C file handle.
        // The stream doesn't own the handle.
        [[nodiscard]] static Output FileHandle(FILE *handle, capacity_t capacity = default_capacity)
        {
            return Output(STR("File handle ", ((void *)handle)),
                [handle](const Output &object, const std::uint8_t *data, std::size_t size)
                {
                    if (!std::fwrite(data, size, 1, handle))
                        Program::Error(object.GetExceptionPrefix() + "Unable to write to file.");
                },
                capacity);
        }

        // Constructs a stream bound to a sequential container.
        // It should work at least with strings and vectors of `char` and `std::uint8_t`.
        template <
            typename T,
            CHECK_EXPR(std::declval<T&>().insert(std::declval<T&>().end(), (const std::uint8_t *)0, (const std::uint8_t *)0))
        >
        [[nodiscard]] static Output Container(T &container, capacity_t capacity = default_capacity)
        {
            return Output(STR("Container at ", ((void *)&container)),
                [&container](const Output &, const std::uint8_t *data, std::size_t size)
                {
                    container.insert(container.end(), data, data + size);
                },
                capacity);
        }

        [[nodiscard]] explicit operator bool() const
        {
            return bool(data.buffer);
        }

        // Returns a name of the data source the stream is bound to.
        [[nodiscard]] std::string GetTarget() const
        {
            return data.name;
        }

        // Does nothing if a style is already selected, unless `force` is true.
        Output &WantExceptionPrefixStyle(ExceptionPrefixStyle style, bool force = false)
        {
            if (!data.exception_prefix_style || force)
                data.exception_prefix_style = style;

            return *this;
        }

        // Returns `with_target_name` by default.
        [[nodiscard]] ExceptionPrefixStyle GetExceptionPrefixStyle() const
        {
            return data.exception_prefix_style.value_or(with_target_name);
        }

        // Uses `GetLocationString` to construct a prefix for exception messages.
        // Prefix format can be changed using `WantExceptionPrefixStyle`.
        [[nodiscard]] std::string GetExceptionPrefix() const
        {
            auto style = GetExceptionPrefixStyle();
            if (style & with_target_name)
            {
                if (*this)
                    return FMT("In an output stream bound to `{}`:\n", GetTarget());
                else
                    return "In a null output stream:\n";
            }

            return "";
        }

        // Flushes the stream.
        // Normally you don't need to do it manually, except before destroying the stream.
        // Explicit flush before destroying is needed because it can throw, and doing so from the destructor
        // is a bad idea (and silently ignoring the exception isn't good too).
        // In a debug build, destroying an unflushed stream raises an assertion;
        // in a release build it's flushed automatically, ignoring any possible exceptions.
        void Flush()
        {
            if (data.buffer_pos > 0)
            {
                data.flush(*this, data.buffer.get(), data.buffer_pos);
                data.buffer_pos = 0;
            }
            else if (!*this)
            {
                Program::Error(GetExceptionPrefix() + "Attempt to flush a null stream.");
            }
        }

        // Writes a single byte.
        Output &WriteByte(std::uint8_t byte)
        {
            NeedBufferSpace();
            data.buffer[data.buffer_pos++] = byte;
            return *this;
        }
        Output &WriteChar(char ch)
        {
            return WriteByte(ch);
        }

        // Writes a single byte several times.
        Output &WriteByte(std::uint8_t byte, std::size_t repeat)
        {
            while (repeat-- > 0)
                WriteByte(byte);
            return *this;
        }
        Output &WriteChar(char ch, std::size_t repeat)
        {
            return WriteByte(ch, repeat);
        }

        // Writes a single UTF8 character.
        Output &WriteUnicodeChar(Unicode::Char ch)
        {
            char buf[Unicode::max_char_len];
            int len = Unicode::Encode(ch, buf);
            for (int i = 0; i < len; i++)
                WriteByte(buf[i]);
            return *this;
        }

        // Writes several bytes.
        Output &WriteBytes(const std::uint8_t *ptr, std::size_t size)
        {
            // If there is a free space in the buffer, fill it.
            std::size_t segment_size = std::min(data.buffer_capacity - data.buffer_pos, size);
            std::copy_n(ptr, segment_size, data.buffer.get() + data.buffer_pos);
            data.buffer_pos += segment_size;
            ptr += segment_size;
            size -= segment_size;

            // If we have no more data to write, stop.
            if (size == 0)
                return *this;

            Flush();

            // If the remaining data fits in the buffer, put it there and stop.
            // Note the `<` instead of `<=`, there is no point in putting the data in the buffer in that case.
            if (size < data.buffer_capacity)
            {
                std::copy_n(ptr, size, data.buffer.get());
                data.buffer_pos = size;
                return *this;
            }

            // If there is too much data, bypass the buffer and flush it directly.
            data.flush(*this, ptr, size);
            return *this;
        }
        Output &WriteString(const char *ptr, std::size_t size)
        {
            return WriteBytes(reinterpret_cast<const std::uint8_t *>(ptr), size);
        }

        // Writes a string.
        // The null-terminator is not written.
        Output &WriteString(const char *string)
        {
            return WriteString(string, std::strlen(string));
        }
        Output &WriteString(const std::string &string)
        {
            return WriteString(string.data(), string.size());
        }

        // Writes an arithmetic value with a specified byte order.
        template <typename T>
        Output &WriteWithByteOrder(ByteOrder::Order order, std::type_identity_t<T> value)
        {
            ByteOrder::Convert(value, order);
            return WriteBytes(reinterpret_cast<const std::uint8_t *>(&value), sizeof value);
        }
        template <typename T>
        Output &WriteLittle(std::type_identity_t<T> value)
        {
            return WriteWithByteOrder<T>(ByteOrder::little, value);
        }
        template <typename T>
        Output &WriteBig(std::type_identity_t<T> value)
        {
            return WriteWithByteOrder<T>(ByteOrder::big, value);
        }
        template <typename T>
        Output &WriteNative(std::type_identity_t<T> value)
        {
            return WriteWithByteOrder<T>(ByteOrder::native, value);
        }

        // Writes a sequence of arithmetic values with a specified byte order.
        template <typename T>
        Output &WriteWithByteOrder(ByteOrder::Order order, const std::type_identity_t<T> *ptr, std::size_t count)
        {
            for (std::size_t i = 0; i < count; i++)
                WriteWithByteOrder<T>(order, ptr[i]);
            return *this;
        }
        template <typename T>
        Output &WriteLittle(const std::type_identity_t<T> *ptr, std::size_t count)
        {
            return WriteWithByteOrder<T>(ByteOrder::little, ptr, count);
        }
        template <typename T>
        Output &WriteBig(const std::type_identity_t<T> *ptr, std::size_t count)
        {
            return WriteWithByteOrder<T>(ByteOrder::big, ptr, count);
        }
        template <typename T>
        Output &WriteNative(const std::type_identity_t<T> *ptr, std::size_t count)
        {
            return WriteWithByteOrder<T>(ByteOrder::native, ptr, count);
        }

        // An output iterator for a stream.
        class OutputIterator
        {
            Output *target = 0;

          public:
            OutputIterator() {}
            OutputIterator(Output &target) : target(&target) {}

            // `std::back_insert_iterator` uses the same aliases.
            using iterator_category = std::output_iterator_tag;
            using value_type        = void;
            using difference_type   = void;
            using pointer           = void;
            using reference         = void;

            // Those return the object itself.
            // Kinda weird, but `std::back_insert_iterator` does the same thing.
            [[nodiscard]] OutputIterator &operator*() {return *this;}
            OutputIterator &operator++() {return *this;}
            OutputIterator &operator++(int) {return *this;}

            OutputIterator &operator=(char ch)
            {
                target->WriteByte(ch);
                return *this;
            }
        };
        // Returns an output iterator for the stream.
        [[nodiscard]] OutputIterator GetOutputIterator()
        {
            return OutputIterator(*this);
        }
    };
}
