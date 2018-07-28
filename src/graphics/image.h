#ifndef GRAPHICS_H_INCLUDED
#define GRAPHICS_H_INCLUDED

#include <vector>
#include <utility>

#include "errors.h"
#include "mat.h"
#include "memory_file.h"

#include <stb_image.h>
#include <stb_image_write.h>

namespace Graphics
{
    class Image
    {
        ivec2 size = ivec2(0);
        std::vector<u8vec4> data;

      public:
        enum Format {png, tga};
        enum FlipMode {no_flip, flip_y};

        Image() {}
        Image(ivec2 size, const uint8_t *bytes = 0) : size(size)
        {
            data = std::vector<u8vec4>((u8vec4 *)bytes, (u8vec4 *)bytes + size.prod());
        }
        Image(MemoryFile file, FlipMode flip_mode = no_flip) // Throws on failure.
        {
            stbi_set_flip_vertically_on_load(flip_mode == flip_y);
            uint8_t *bytes = 0;
            try
            {
                ivec2 img_size;
                bytes = stbi_load_from_memory(file.data(), file.size(), &img_size.x, &img_size.y, 0, 4);
                if (!bytes)
                    Program::Error("Unable to parse image: ", file.name());
                *this = Image(img_size, bytes);
                stbi_image_free(bytes);
                bytes = 0;
            }
            catch (...)
            {
                if (bytes)
                    stbi_image_free(bytes);
                throw;
            }
        }

        Image(const Image &other) : size(other.size), data(other.data) {}
        Image &operator=(const Image &other)
        {
            size = other.size;
            data = other.data;
            return *this;
        }
        Image(Image &&other) : size(std::exchange(other.size, ivec2(0))), data(std::move(other.data)) {}
        Image &operator=(Image &&other)
        {
            if (&other == this)
                return *this;
            size = std::exchange(other.size, ivec2(0));
            data = std::move(other.data);
            return *this;
        }

        explicit operator bool() const {return data.size() > 0;}

        ivec2 Size() const {return size;}
        const u8vec4 *Data() const {return data.data();}

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
    };
}

#endif
