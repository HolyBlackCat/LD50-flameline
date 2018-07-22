#ifndef ARCHIVE_H_INCLUDED
#define ARCHIVE_H_INCLUDED

#include <cstdint>
#include <cstddef>

namespace Archive
{
    namespace Raw // Those are thin wrappers around zlib.
    {
        [[nodiscard]] std::size_t MaxCompressedSize(const uint8_t *src_begin, const uint8_t *src_end); // Determines max destination buffer size.
        [[nodiscard]] uint8_t *Compress(const uint8_t *src_begin, const uint8_t *src_end, uint8_t *dst_begin, uint8_t *dst_end); // Compresses and returns compressed data end. Throws on failure.
        void Uncompress(const uint8_t *src_begin, const uint8_t *src_end, uint8_t *dst_begin, uint8_t *dst_end); // Decompresses. Throws on failure. Also throws if buffer is too large.
    }

    // Those functions prefix compressed data with size.

    [[nodiscard]] std::size_t MaxCompressedSize(const uint8_t *src_begin, const uint8_t *src_end); // Determines max destination buffer size.
    [[nodiscard]] uint8_t *Compress(const uint8_t *src_begin, const uint8_t *src_end, uint8_t *dst_begin, uint8_t *dst_end); // Compresses and returns compressed data end. Throws on failure.
    [[nodiscard]] std::size_t UncompressedSize(const uint8_t *src_begin, const uint8_t *src_end); // Extracts size from decompressed data. Throws on failure.
    void Uncompress(const uint8_t *src_begin, const uint8_t *src_end, uint8_t *dst_begin); // Decompresses. Throws on failure. The buffer must have size returned by `UncompressedSize()`.
}

#endif
