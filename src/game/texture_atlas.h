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

    Image Get(const std::string &name) const
    {
        auto it = desc.images.find(name);
        if (it == desc.images.end())
            Program::Error("No image `", name, "` in texture atlas for `", source_dir, "`.");

        Image ret;
        ret.pos = it->second.pos;
        ret.size = it->second.size;
        return ret;
    }
};
