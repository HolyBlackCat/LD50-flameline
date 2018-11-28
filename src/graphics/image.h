#pragma once

#include <algorithm>
#include <vector>
#include <utility>

#include "program/errors.h"
#include "utils/finally.h"
#include "utils/mat.h"
#include "utils/memory_file.h"

#include <stb_image.h>
#include <stb_image_write.h>

namespace Graphics
{
    class Image
    {
        // Note that moved-from instance is left in an invalid (yet destructable) state.

        ivec2 size = ivec2(0);
        std::vector<u8vec4> data;

      public:
        enum Format {png, tga};
        enum FlipMode {no_flip, flip_y};

        Image() {}
        Image(ivec2 size, const uint8_t *bytes = 0) : size(size) // If `bytes == 0`, then the image will be filled with transparent black.
        {
            if (bytes)
                data = std::vector<u8vec4>((u8vec4 *)bytes, (u8vec4 *)bytes + size.prod());
            else
                data = std::vector<u8vec4>(size.prod());
        }
        Image(ivec2 size, u8vec4 color) : size(size)
        {
            data = std::vector<u8vec4>(size.prod(), color);
        }
        Image(MemoryFile file, FlipMode flip_mode = no_flip) // Throws on failure.
        {
            stbi_set_flip_vertically_on_load(flip_mode == flip_y);
            ivec2 img_size;
            uint8_t *bytes = stbi_load_from_memory(file.data(), file.size(), &img_size.x, &img_size.y, 0, 4);
            if (!bytes)
                Program::Error("Unable to parse image: ", file.name());
            FINALLY( stbi_image_free(bytes); )
            *this = Image(img_size, bytes);
        }

        explicit operator bool() const {return data.size() > 0;}

        const u8vec4 *Pixels() const {return data.data();}
        const uint8_t *Data() const {return (const uint8_t *)Pixels();}
        ivec2 Size() const {return size;}

        bool PointInBounds(ivec2 point) const
        {
            return (point >= 0).all() && (point < size).all();
        }
        bool RectInBounds(ivec2 rect_pos, ivec2 rect_size) const
        {
            return (rect_pos >= 0).all() && (rect_pos + rect_size <= size).all() && (rect_size >= 0).all();
        }

        void Save(std::string file_name, Format format = png) // Throws on failure.
        {
            if (!*this)
                Program::Error("Attempt to save an empty image to a file.");

            int ok = 0;
            switch (format)
            {
              case png:
                ok = stbi_write_png(file_name.c_str(), size.x, size.y, 4, data.data(), 0);
                break;
              case tga:
                ok = stbi_write_tga(file_name.c_str(), size.x, size.y, 4, data.data());
                break;
            }

            if (!ok)
                Program::Error("Unable to write image to file: ", file_name);
        }

        u8vec4 &UnsafeAt(ivec2 pos)
        {
            return const_cast<u8vec4 &>(std::as_const(*this).UnsafeAt(pos));
        }
        const u8vec4 &UnsafeAt(ivec2 pos) const
        {
            return data[pos.x + pos.y * size.x];
        }

        u8vec4 TryGet(ivec2 pos) const // Returns transparent black if out of range.
        {
            if (PointInBounds(pos))
                return UnsafeAt(pos);
            else
                return u8vec4(0);
        }
        void TrySet(ivec2 pos, u8vec4 color)
        {
            if (PointInBounds(pos))
                UnsafeAt(pos) = color;
        }

        void UnsafeFill(ivec2 rect_pos, ivec2 rect_size, u8vec4 color)
        {
            for (int y = rect_pos.y; y < rect_pos.y + rect_size.y; y++)
            for (int x = rect_pos.x; x < rect_pos.x + rect_size.x; x++)
                UnsafeAt(ivec2(x,y)) = color;
        }

        void UnsafeDrawImage(const Image &other, ivec2 pos) // Copies other image into this image, at specified location.
        {
            for (int y = 0; y < other.Size().y; y++)
            {
                auto source_address = &other.UnsafeAt(ivec2(0,y));
                std::copy(source_address, source_address + other.Size().x, &UnsafeAt(pos.add_y(y)));
            }
        }
    };
}
