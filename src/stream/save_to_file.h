#pragma once

#include <cstdio>
#include <cstdint>
#include <iterator>
#include <string>
#include <memory>
#include <utility>

#include "macros/finally.h"
#include "program/errors.h"
#include "stream/better_fopen.h"
#include "stream/utils.h"
#include "utils/archive.h"

namespace Stream
{
    enum SaveMode
    {
        overwrite,
        append,
    };

    [[nodiscard]] inline const char *SaveModeStringRepresentation(SaveMode mode)
    {
        switch (mode)
        {
            case overwrite: return "wb";
            case append:    return "ab";
        }

        return "wb";
    }


    // Saves a block of memory to a file. Throws on failure.
    inline void SaveFile(std::string file_name, const std::uint8_t *begin, const std::uint8_t *end, SaveMode mode = overwrite)
    {
        FILE *file = better_fopen(file_name.c_str(), SaveModeStringRepresentation(mode));
        if (!file)
            Program::Error("Unable to open file `", file_name, "` for writing.");
        FINALLY( std::fclose(file); )
        if (!std::fwrite(begin, end - begin, 1, file))
            Program::Error("Unable to write to file `", file_name, "`.");
    }

    // Saves a block of memory to a file. Throws on failure.
    inline void SaveFile(std::string file_name, const char *begin, const char *end, SaveMode mode = overwrite)
    {
        SaveFile(std::move(file_name), reinterpret_cast<const std::uint8_t *>(begin), reinterpret_cast<const std::uint8_t *>(end), mode);
    }

    // Saves a container to a file. Throws on failure.
    template <typename T, CHECK_TYPE(impl::detect_flat_byte_container<T>)>
    void SaveFile(std::string file_name, const T &container, SaveMode mode = overwrite)
    {
        const std::uint8_t *ptr = reinterpret_cast<const std::uint8_t *>(std::data(container));
        SaveFile(std::move(file_name), ptr, ptr + std::size(container), mode);
    }


    // Saves a block of memory to a file, in a compressed form (see `archive.h` for details). Throws on failure.
    inline void SaveFileCompressed(std::string file_name, const std::uint8_t *begin, const std::uint8_t *end)
    {
        auto buffer_size = Archive::MaxCompressedSize(begin, end);
        auto buffer = std::make_unique<std::uint8_t[]>(buffer_size);
        auto compressed_end = Archive::Compress(begin, end, buffer.get(), buffer.get() + buffer_size);
        SaveFile(file_name, buffer.get(), compressed_end);
    }

    // Saves a block of memory to a file, in a compressed form (see `archive.h` for details). Throws on failure.
    inline void SaveFileCompressed(std::string file_name, const char *begin, const char *end)
    {
        SaveFileCompressed(std::move(file_name), reinterpret_cast<const std::uint8_t *>(begin), reinterpret_cast<const std::uint8_t *>(end));
    }

    // Saves a container to a file, in a compressed form (see `archive.h` for details). Throws on failure.
    template <typename T, CHECK_TYPE(impl::detect_flat_byte_container<T>)>
    void SaveFileCompressed(std::string file_name, const T &container)
    {
        const std::uint8_t *ptr = reinterpret_cast<const std::uint8_t *>(std::data(container));
        SaveFileCompressed(std::move(file_name), ptr, ptr + std::size(container));
    }
}
