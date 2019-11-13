#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

#include "program/errors.h"
#include "utils/archive.h"
#include "utils/finally.h"
#include "utils/strings.h"

namespace Stream
{
    class MemoryFile
    {
        // A copy-on-write

        struct Data
        {
            std::unique_ptr<std::uint8_t[]> storage;

            const std::uint8_t *begin = 0, *end = 0;
            bool extra_null_terminator = false; // If this is `true`, there is an extra null terminator past the `end`.

            std::string name;
        };

        std::shared_ptr<Data> ref;

      public:
        MemoryFile() {}

        MemoryFile(std::string file_name)
        {
            *this = file(file_name);
        }
        MemoryFile(const char *file_name) // This allows implicit conversions from string literals.
        {
            *this = file(file_name);
        }
        MemoryFile(const std::uint8_t *begin, const std::uint8_t *end) // Doesn't copy the data (calls `mem_reference()`) under the hood.
        {
            *this = mem_reference(begin, end);
        }
        MemoryFile(const char *begin, const char *end) // Doesn't copy the data (calls `mem_reference()`) under the hood.
        {
            *this = mem_reference(begin, end);
        }

        // Stores a reference to an existing memory block.
        [[nodiscard]] static MemoryFile mem_reference(const std::uint8_t *begin, const std::uint8_t *end)
        {
            MemoryFile ret;
            ret.ref = std::make_shared<Data>();

            ret.ref->begin = begin;
            ret.ref->end = end;

            ret.ref->name = Str("Reference to ", end - begin, " bytes at 0x", std::hex, begin);

            return ret;
        }

        // Stores a reference to an existing memory block.
        [[nodiscard]] static MemoryFile mem_reference(const char *begin, const char *end)
        {
            return mem_reference(reinterpret_cast<const std::uint8_t *>(begin), reinterpret_cast<const std::uint8_t *>(end));
        }

        // Copies an existing memory block, adds a null-terminator.
        [[nodiscard]] static MemoryFile mem_copy(const std::uint8_t *begin, const std::uint8_t *end)
        {
            auto size = end - begin;

            MemoryFile ret;
            ret.ref = std::make_shared<Data>();

            ret.ref->storage = std::make_unique<std::uint8_t[]>(size+1); // 1 extra byte for the null-terminator.
            std::copy(begin, end, ret.ref->storage.get());
            ret.ref->storage[size] = '\0';

            ret.ref->begin = ret.ref->storage.get();
            ret.ref->end = ret.ref->begin + size;
            ret.ref->extra_null_terminator = true;

            ret.ref->name = Str("Copy of ", size-1, " bytes from 0x", std::hex, begin);

            return ret;
        }

        // Copies an existing memory block, adds a null-terminator.
        [[nodiscard]] static MemoryFile mem_copy(const char *begin, const char *end)
        {
            return mem_copy(reinterpret_cast<const std::uint8_t *>(begin), reinterpret_cast<const std::uint8_t *>(end));
        }

        // Loads an entire file to memory, adds a null-terminator.
        [[nodiscard]] static MemoryFile file(std::string file_name)
        {
            MemoryFile ret;
            ret.ref = std::make_shared<Data>();

            FILE *file = std::fopen(file_name.c_str(), "rb");
            if (!file)
                Program::Error("Unable to open file `", file_name, "`.");
            FINALLY( std::fclose(file); )

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
            ret.ref->name = file_name;

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
        [[nodiscard]] MemoryFile uncompress() const
        {
            if (!ref)
                return {};

            MemoryFile ret;

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
        [[nodiscard]] MemoryFile null_terminate() const
        {
            if (!ref)
                return {};

            if (is_null_terminated())
                return *this;

            MemoryFile ret;
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

    // Saves a block of memory to a file.
    // Throws on failure.
    inline void SaveFile(std::string file_name, const std::uint8_t *begin, const std::uint8_t *end)
    {
        FILE *file = std::fopen(file_name.c_str(), "wb");
        if (!file)
            Program::Error("Unable to open file `", file_name, "` for writing.");
        FINALLY( std::fclose(file); )
        if (!std::fwrite(begin, end - begin, 1, file))
            Program::Error("Unable to write to file `", file_name, "`.");
    }

    // Saves a block of memory to a file.
    // Throws on failure.
    inline void SaveFile(std::string file_name, const std::string &string) // Throws on failure.
    {
        auto ptr = reinterpret_cast<const std::uint8_t *>(string.c_str());
        SaveFile(file_name, ptr, ptr + string.size());
    }

    // Saves a block of memory to a file, in a compressed form (see `archive.h` for details).
    // Throws on failure.
    inline void SaveFileCompressed(std::string file_name, const std::uint8_t *begin, const std::uint8_t *end) // Throws on failure.
    {
        auto buffer_size = Archive::MaxCompressedSize(begin, end);
        auto buffer = std::make_unique<std::uint8_t[]>(buffer_size);
        auto compressed_end = Archive::Compress(begin, end, buffer.get(), buffer.get() + buffer_size);
        SaveFile(file_name, buffer.get(), compressed_end);
    }

    // Saves a block of memory to a file, in a compressed form (see `archive.h` for details).
    // Throws on failure.
    inline void SaveFileCompressed(std::string file_name, const std::string &string) // Throws on failure.
    {
        auto ptr = reinterpret_cast<const std::uint8_t *>(string.c_str());
        SaveFileCompressed(file_name, ptr, ptr + string.size());
    }
}
