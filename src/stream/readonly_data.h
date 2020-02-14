#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "macros/check.h"
#include "macros/finally.h"
#include "program/errors.h"
#include "stream/better_fopen.h"
#include "stream/utils.h"
#include "strings/common.h"
#include "utils/archive.h"

namespace Stream
{
    class ReadOnlyData
    {
        // A copy-on-write immutable data storage. It may or may not own the data.

        struct Data
        {
            std::unique_ptr<std::uint8_t[]> storage;

            const std::uint8_t *begin = 0, *end = 0;
            bool extra_null_terminator = false; // If this is `true`, there is an extra null terminator past the `end`.

            std::string name;
        };

        std::shared_ptr<Data> ref;

      public:
        ReadOnlyData() {}

        ReadOnlyData(std::string file_name)
        {
            *this = file(std::move(file_name));
        }
        ReadOnlyData(const char *file_name) // This allows implicit conversions from string literals.
        {
            *this = file(file_name);
        }

        // Stores a reference to an existing memory block.
        [[nodiscard]] static ReadOnlyData mem_reference(const std::uint8_t *begin, const std::uint8_t *end)
        {
            ReadOnlyData ret;
            ret.ref = std::make_shared<Data>();

            ret.ref->begin = begin;
            ret.ref->end = end;

            ret.ref->name = Str("Reference to ", end - begin, " bytes at 0x", std::hex, std::uintptr_t(begin));

            return ret;
        }
        // Stores a reference to an existing memory block.
        [[nodiscard]] static ReadOnlyData mem_reference(const char *begin, const char *end)
        {
            return mem_reference(reinterpret_cast<const std::uint8_t *>(begin), reinterpret_cast<const std::uint8_t *>(end));
        }
        // Stores a reference to an existing contaner.
        template <typename T, CHECK_TYPE(impl::detect_flat_byte_container<T>)>
        [[nodiscard]] static ReadOnlyData mem_reference(const T &container)
        {
            auto pointer = reinterpret_cast<const std::uint8_t *>(std::data(container));
            return mem_reference(pointer, pointer + std::size(container));
        }

        template <typename F, CHECK_EXPR(std::declval<F>()((std::uint8_t *)nullptr))>
        [[nodiscard]] static ReadOnlyData copy_from_function(std::string name, std::size_t size, F &&func) // `func` is `void func(std::uint8_t *)`.
        {
            ReadOnlyData ret;
            ret.ref = std::make_shared<Data>();

            ret.ref->storage = std::make_unique<std::uint8_t[]>(size+1); // Plus one byte for a null-terminator.
            std::forward<F>(func)(ret.ref->storage.get());
            ret.ref->storage[size] = '\0';

            ret.ref->begin = ret.ref->storage.get();
            ret.ref->end = ret.ref->storage.get() + size;
            ret.ref->extra_null_terminator = true;

            ret.ref->name = std::move(name);
            return ret;
        }

        // Copies an existing memory block, adds a null-terminator.
        [[nodiscard]] static ReadOnlyData mem_copy(const std::uint8_t *begin, const std::uint8_t *end)
        {
            auto size = end - begin;

            ReadOnlyData ret;
            ret.ref = std::make_shared<Data>();

            ret.ref->storage = std::make_unique<std::uint8_t[]>(size+1); // 1 extra byte for the null-terminator.
            std::copy(begin, end, ret.ref->storage.get());
            ret.ref->storage[size] = '\0';

            ret.ref->begin = ret.ref->storage.get();
            ret.ref->end = ret.ref->begin + size;
            ret.ref->extra_null_terminator = true;

            ret.ref->name = Str("Copy of ", size-1, " bytes from 0x", std::hex, std::uintptr_t(begin));

            return ret;
        }
        // Copies an existing memory block, adds a null-terminator.
        [[nodiscard]] static ReadOnlyData mem_copy(const char *begin, const char *end)
        {
            return mem_copy(reinterpret_cast<const std::uint8_t *>(begin), reinterpret_cast<const std::uint8_t *>(end));
        }
        // Copies an existing container, adds a null-terminator.
        template <typename T, CHECK_TYPE(impl::detect_flat_byte_container<T>)>
        [[nodiscard]] static ReadOnlyData mem_copy(const T &container)
        {
            auto pointer = reinterpret_cast<const std::uint8_t *>(std::data(container));
            return mem_copy(pointer, pointer + std::size(container));
        }

        // Loads an entire file to memory, adds a null-terminator.
        [[nodiscard]] static ReadOnlyData file(std::string file_name)
        {
            ReadOnlyData ret;
            ret.ref = std::make_shared<Data>();

            FILE *file = better_fopen(file_name.c_str(), "rb");
            if (!file)
                Program::Error("Unable to open file `", file_name, "`.");
            FINALLY( std::fclose(file); )

            std::setbuf(file, 0); // This can fail, but we don't care about it.

            std::fseek(file, 0, SEEK_END);
            auto size = std::ftell(file);
            std::fseek(file, 0, SEEK_SET);

            if (std::ferror(file) || size == EOF)
                Program::Error("Unable to get size of file `", file_name, "`.");

            ret.ref->storage = std::make_unique<std::uint8_t[]>(size+1); // 1 extra byte for the null-terminator.
            if (size > 1 && !std::fread(ret.ref->storage.get(), size, 1, file))
                Program::Error("Unable to read from file `", file_name, "`.");
            ret.ref->storage[size] = '\0';

            ret.ref->begin = ret.ref->storage.get();
            ret.ref->end = ret.ref->begin + size;
            ret.ref->extra_null_terminator = true;
            ret.ref->name = std::move(file_name);

            return ret;
        }

        [[nodiscard]] explicit operator bool() const
        {
            return bool(ref);
        }

        // Returns a description of the target.
        [[nodiscard]] std::string name() const
        {
            if (!ref)
                return "";
            return ref->name;
        }

        // Returns a pointer to the beginning of the storage.
        [[nodiscard]] const std::uint8_t *data() const
        {
            return begin();
        }
        // Returns a pointer to the beginning of the storage.
        [[nodiscard]] const char *data_char() const
        {
            return begin_char();
        }
        // Returns the data size, excluding the extra null-terminator if it's present.
        [[nodiscard]] std::size_t size() const
        {
            return end() - begin();
        }

        // Returns a pointer to the beginning of the storage.
        [[nodiscard]] const std::uint8_t *begin() const
        {
            if (!ref)
                return 0;
            return ref->begin;
        }
        // Returns a pointer to the beginning of the storage.
        [[nodiscard]] const char *begin_char() const
        {
            if (!ref)
                return 0;
            return reinterpret_cast<const char *>(ref->begin);
        }
        // Returns a pointer to the end of the storage, excluding the extra null-terminator if it's present.
        [[nodiscard]] const std::uint8_t *end() const
        {
            if (!ref)
                return 0;
            return ref->end;
        }
        // Returns a pointer to the end of the storage, excluding the extra null-terminator if it's present.
        [[nodiscard]] const char *end_char() const
        {
            if (!ref)
                return 0;
            return reinterpret_cast<const char *>(ref->end);
        }

        // Returns an uncompressed copy of the file.
        // The data is assumed to be size-prefixed, see `archive.h` for the details.
        [[nodiscard]] ReadOnlyData uncompress() const
        {
            if (!ref)
                return {};

            ReadOnlyData ret;

            try
            {
                auto size = Archive::UncompressedSize(ref->begin, ref->end);

                ret.ref = std::make_shared<Data>();

                ret.ref->storage = std::make_unique<std::uint8_t[]>(size+1); // 1 extra byte for a null-terminator.
                ret.ref->begin = ret.ref->storage.get();
                ret.ref->end = ret.ref->begin + size;
                ret.ref->extra_null_terminator = true;

                Archive::Uncompress(ref->begin, ref->end, ret.ref->storage.get());
                ret.ref->storage[size] = '\0';

                ret.ref->name = ref->name + " (uncompressed)";
            }
            catch (...)
            {
                Program::Error("Unable to uncompress: ", ref->name);
            }

            return ret;
        }

        // Checks for a null-terminator, either a natural or an automatically-inserted one.
        // If the null-terminator was inserted automatically, it's not going to be considered a part of the data by `begin()/end()` and `size()`.
        [[nodiscard]] bool is_null_terminated() const
        {
            if (!ref)
                return false;

            if (ref->extra_null_terminator)
                return true;

            if (size() != 0 && end()[-1] == '\0')
                return true;

            return false;
        }
        // If the null-terminator is missing, copies the data and adds it to the copy.
        [[nodiscard]] ReadOnlyData null_terminate() const
        {
            if (!ref)
                return {};

            if (is_null_terminated())
                return *this;

            ReadOnlyData ret;
            ret.ref = std::make_shared<Data>();

            ret.ref->storage = std::make_unique<std::uint8_t[]>(size()+1);
            std::copy(begin(), end(), ret.ref->storage.get());
            ret.ref->storage[size()] = '\0';

            ret.ref->begin = ret.ref->storage.get();
            ret.ref->end = ret.ref->begin + size();
            ret.ref->extra_null_terminator = true;

            ret.ref->name = name();
            return ret;
        }

        // Returns a pointer to the beginning of the file.
        // Adds a null-terminator to the data if it was missing.
        [[nodiscard]] const char *string()
        {
            if (!ref)
                return "";

            if (!is_null_terminated())
                *this = null_terminate();

            return (const char *)data();
        }
    };
}
