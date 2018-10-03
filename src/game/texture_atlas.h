#pragma once

#include <ctime>
#include <map>
#include <string>
#include <vector>
#include <utility>

#include "graphics/image.h"
#include "program/errors.h"
#include "reflection/complete.h"
#include "utils/filesystem.h"
#include "utils/mat.h"
#include "utils/memory_file.h"
#include "utils/strings.h"

class TextureAtlas
{
    ReflectStruct(ImageDesc,(
        (ivec2)(pos),
        (ivec2)(size),
        (using _refl_structure_tuple_tag = void;), // Enable terse string representation.
    ))

    ReflectStruct(Desc,(
        (std::map<std::string, ImageDesc>)(images),
    ))

    Graphics::Image image;
    Desc desc;
    std::string source_dir;

  public:
    struct Image
    {
        ivec2 pos = ivec2(0);
        ivec2 size = ivec2(0);

        Image() {}
    };

    class ImageList
    {
        friend class TextureAtlas;

        std::vector<Image> list;

      public:
        ImageList() {}

        // [] wraps around for easier animation.
        Image &operator[](int index)
        {
            return const_cast<Image &>(std::as_const(*this)[index]);
        }
        const Image &operator[](int index) const
        {
            return list[mod_ex(index, int(list.size()))];
        }
    };

    TextureAtlas() {}

    // Pass empty string as `source_dir` to disallow regeneration.
    TextureAtlas(ivec2 target_size, const std::string &source_dir, const std::string &out_image_file, const std::string &out_desc_file, bool add_gaps = 1);

    const std::string SourceDirectory() const
    {
        return source_dir;
    }

    const Graphics::Image &GetImage() const
    {
        return image;
    }

    bool GetOpt(const std::string &name, Image &target) const // Returns false if no such image.
    {
        auto it = desc.images.find(name);
        if (it == desc.images.end())
            return 0;

        target.pos = it->second.pos;
        target.size = it->second.size;
        return 1;
    }

    Image Get(const std::string &name) const
    {
        Image ret;
        if (!GetOpt(name, ret))
            Program::Error("No image `", name, "` in texture atlas for `", source_dir, "`.");
        return ret;
    }
    ImageList GetList(const std::string &prefix, int first_index, const std::string &suffix, int count = -1) const
    {
        ImageList ret;

        int offset = 0;
        while (offset != count)
        {
            int index = first_index + offset;
            std::string name = Str(prefix, index, suffix);

            Image image;
            if (!GetOpt(name, image))
            {
                if (count < 0)
                    break;
                Program::Error("Image list `", prefix, '#', suffix, "` from texture atlas for `", source_dir, "` has no image with index ", index, ".");
            }
            ret.list.push_back(std::move(image));

            offset++;
        }

        return ret;
    }
};
