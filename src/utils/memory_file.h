#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

#include "archive.h"
#include "finally.h"
#include "strings.h"
#include "program/errors.h"

class MemoryFile
{
    struct Data
    {
        std::unique_ptr<std::uint8_t[]> storage;
        const std::uint8_t *begin = 0, *end = 0;
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

    [[nodiscard]] static MemoryFile mem_reference(const std::uint8_t *begin, const std::uint8_t *end)
    {
        MemoryFile ret;
        ret.ref = std::make_shared<Data>();

        ret.ref->begin = begin;
        ret.ref->end = end;

        ret.ref->name = Str("Reference to ", end - begin, " bytes at 0x", std::hex, begin);
        return ret;
    }
    [[nodiscard]] static MemoryFile mem_copy(const std::uint8_t *begin, const std::uint8_t *end)
    {
        auto size = end - begin + 1; // 1 extra byte for the null-terminator.

        MemoryFile ret;
        ret.ref = std::make_shared<Data>();

        ret.ref->storage = std::make_unique<std::uint8_t[]>(size);
        std::copy(begin, end, ret.ref->storage.get());
        ret.ref->storage[size-1] = '\0';

        ret.ref->begin = ret.ref->storage.get();
        ret.ref->end = ret.ref->begin + size;

        ret.ref->name = Str("Copy of ", size-1, " bytes from 0x", std::hex, begin);
        return ret;
    }
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

        size++; // 1 extra byte for the null-terminator.

        ret.ref->storage = std::make_unique<std::uint8_t[]>(size);
        if (size > 1 && !std::fread(ret.ref->storage.get(), size-1, 1, file)) // `-1` leaves a free byte for the null-terminator.
            Program::Error("Unable to read from file `", file_name, "`.");
        ret.ref->storage[size-1] = '\0';

        ret.ref->begin = ret.ref->storage.get();
        ret.ref->end = ret.ref->begin + size;
        ret.ref->name = file_name;

        return ret;
    }

    [[nodiscard]] explicit operator bool() const
    {
        return bool(ref);
    }

    [[nodiscard]] std::string name() const
    {
        if (!ref)
            return "";
        return ref->name;
    }

    [[nodiscard]] const std::uint8_t *data() const
    {
        return begin();
    }
    [[nodiscard]] std::size_t size() const
    {

        return end() - begin() - is_null_terminated();
    }

    [[nodiscard]] const std::uint8_t *begin() const
    {
        if (!ref)
            return 0;
        return ref->begin;
    }
    [[nodiscard]] const std::uint8_t *end() const
    {
        if (!ref)
            return 0;
        return ref->end;
    }

    [[nodiscard]] MemoryFile uncompress() const
    {
        if (!ref)
            return {};

        MemoryFile ret;

        try
        {
            auto size = Archive::UncompressedSize(ref->begin, ref->end) + 1; // 1 extra byte for a null-terminator.

            ret.ref = std::make_shared<Data>();

            ret.ref->storage = std::make_unique<std::uint8_t[]>(size);
            Archive::Uncompress(ref->begin, ref->end, ret.ref->storage.get());
            ret.ref->storage[size-1] = '\0';

            ret.ref->begin = ret.ref->storage.get();
            ret.ref->end = ret.ref->begin + size;

            ret.ref->name = ref->name + " (uncompressed)";
        }
        catch (...)
        {
            Program::Error("Unable to uncompress: ", ref->name);
        }

        return ret;
    }

    [[nodiscard]] bool is_null_terminated() const
    {
        if (!ref)
            return 0;

        if (begin() == end()) // We can't use `size() == 0` here because `size()` returns size without '\0' (also because it uses `is_null_terminated()`).
            return 0;

        return char(end()[-1]) == '\0';
    }
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
        ret.ref->end = ret.ref->begin + size()+1;

        ret.ref->name = name();
        return ret;
    }

    [[nodiscard]] const char *string()
    {
        if (!ref)
            return "";

        if (!is_null_terminated())
            *this = null_terminate();

        return (const char *)data();
    }


    static void Save(std::string file_name, const std::uint8_t *begin, const std::uint8_t *end) // Throws on failure.
    {
        FILE *file = std::fopen(file_name.c_str(), "wb");
        if (!file)
            Program::Error("Unable to open file `", file_name, "` for writing.");
        FINALLY( std::fclose(file); )
        if (!std::fwrite(begin, end - begin, 1, file))
            Program::Error("Unable to write to file `", file_name, "`.");
    }

    static void SaveCompressed(std::string file_name, const std::uint8_t *begin, const std::uint8_t *end) // Throws on failure.
    {
        auto buffer_size = Archive::MaxCompressedSize(begin, end);
        auto buffer = std::make_unique<std::uint8_t[]>(buffer_size);
        auto compressed_end = Archive::Compress(begin, end, buffer.get(), buffer.get() + buffer_size);
        Save(file_name, buffer.get(), compressed_end);
    }

    static void Save(std::string file_name, const std::string &string) // Throws on failure.
    {
        auto ptr = reinterpret_cast<const std::uint8_t *>(string.c_str());
        Save(file_name, ptr, ptr + string.size());
    }

    static void SaveCompressed(std::string file_name, const std::string &string) // Throws on failure.
    {
        auto ptr = reinterpret_cast<const std::uint8_t *>(string.c_str());
        SaveCompressed(file_name, ptr, ptr + string.size());
    }
};
