#pragma once

#include <ctime>
#include <map>
#include <string>
#include <vector>
#include <utility>

#include "image.h"
#include "program/errors.h"
#include "reflection/complete.h"
#include "utils/filesystem.h"
#include "utils/mat.h"
#include "utils/memory_file.h"
#include "utils/strings.h"

namespace Graphics
{
    ReflectStruct(ImageLocation,(
        (ivec2)(pos),
        (ivec2)(size),
        (using _refl_structure_tuple_tag = void;), // Enable terse string representation.
    ))

    class TextureAtlas
    {
        static constexpr char image_extension[] = ".png";

        ReflectStruct(Desc,(
            (std::map<std::string, ImageLocation>)(images),
        ))

        Image image;
        Desc description;
        std::string image_name;

      public:
        TextureAtlas() {}

        // Pass empty string as `source_dir_name` to disallow regeneration.
        TextureAtlas(ivec2 target_size, const std::string &source_dir_name, const std::string &image_file_name, const std::string &desc_file_name, bool add_gaps = 1);

        const std::string ImageName() const
        {
            return image_name;
        }

        ImageLocation Get(const std::string &name) const
        {
            if (auto it = description.images.find(name); it != description.images.end())
                return it->second;
            else
                Program::Error("No image `", name, "` in texture atlas `", image_name, "`.");
        }
    };
}
