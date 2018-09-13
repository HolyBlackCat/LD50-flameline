#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

#include "archive.h"
#include "strings.h"
#include "program/errors.h"

class MemoryFile
{
    struct Data
    {
        std::unique_ptr<uint8_t[]> storage;
        const uint8_t *begin, *end;
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
    MemoryFile(const uint8_t *begin, const uint8_t *end) // Doesn't copy the data (calls `mem_reference()`) under the hood.
    {
        *this = mem_reference(begin, end);
    }

    [[nodiscard]] static MemoryFile mem_reference(const uint8_t *begin, const uint8_t *end)
    {
        MemoryFile ret;
        ret.ref = std::make_shared<Data>();

        ret.ref->begin = begin;
        ret.ref->end = end;

        ret.ref->name = Str("Reference to ", end - begin, " bytes at 0x", std::hex, begin);
        return ret;
    }
    [[nodiscard]] static MemoryFile mem_copy(const uint8_t *begin, const uint8_t *end)
    {
        auto size = end - begin;

        MemoryFile ret;
        ret.ref = std::make_shared<Data>();

        ret.ref->storage = std::make_unique<uint8_t[]>(size);
        std::copy(begin, end, ret.ref->storage.get());

        ret.ref->begin = ret.ref->storage.get();
        ret.ref->end = ret.ref->begin + size;

        ret.ref->name = Str("Copy of ", size, " bytes from 0x", std::hex, begin);
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

        ret.ref->storage = std::make_unique<uint8_t[]>(size);
        if (!std::fread(ret.ref->storage.get(), size, 1, file))
            Program::Error("Unable to read from file `", file_name, "`.");

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

    [[nodiscard]] const uint8_t *data() const
    {
        return begin();
    }
    [[nodiscard]] std::size_t size() const
    {
        return end() - begin();
    }

    [[nodiscard]] const uint8_t *begin() const
    {
        if (!ref)
            return 0;
        return ref->begin;
    }
    [[nodiscard]] const uint8_t *end() const
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
            auto size = Archive::UncompressedSize(ref->begin, ref->end);

            ret.ref = std::make_shared<Data>();

            ret.ref->storage = std::make_unique<uint8_t[]>(size);
            Archive::Uncompress(ref->begin, ref->end, ret.ref->storage.get());

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

    [[nodiscard]] std::string construct_string() const
    {
        return std::string((char*)begin(), (char*)end());
    }


    static void Save(std::string file_name, const uint8_t *begin, const uint8_t *end) // Throws on failure.
    {
        FILE *file = std::fopen(file_name.c_str(), "wb");
        if (!file)
            Program::Error("Unable to open file for writing: ", file_name);
        FINALLY( std::fclose(file); )
        if (!std::fwrite(begin, end - begin, 1, file))
            Program::Error("Unable to write to file: ", file_name);
    }

    static void SaveCompressed(std::string file_name, const uint8_t *begin, const uint8_t *end) // Throws on failure.
    {
        auto buffer_size = Archive::MaxCompressedSize(begin, end);
        auto buffer = std::make_unique<uint8_t[]>(buffer_size);
        auto compressed_end = Archive::Compress(begin, end, buffer.get(), buffer.get() + buffer_size);
        Save(file_name, buffer.get(), compressed_end);
    }
};
