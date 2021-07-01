#include "texture_atlas.h"

#include <memory>

#include "reflection/full.h"
#include "stream/readonly_data.h"
#include "stream/save_to_file.h"
#include "utils/packing.h"

namespace Graphics
{
    TextureAtlas::TextureAtlas(ivec2 target_size, const std::string &source_dir, const std::string &out_image_file, const std::string &out_desc_file, const std::map<std::string, ivec2> &artifical_regions, bool add_gaps)
        : source_dir(source_dir)
    {
        constexpr int max_nesting_level = 32;

        // Decide if regenrating the atlas should be allowed.
        bool allow_regeneration = !source_dir.empty();

        Filesystem::TreeNode source_tree;

        // If regeneration is allowed, try getting the source directory tree and its modification time.
        if (allow_regeneration)
        {
            // `GetObjectTree` will throw if the source directory doesn't exist.
            source_tree = Filesystem::GetObjectTree(source_dir, max_nesting_level);
            if (source_tree.info.category != Filesystem::directory)
                Program::Error("Texture atlas source location `", source_dir, "` is not a directory.");
        }

        std::time_t image_time_modified = 0;

        { // Get image file modification time. 0 if no file.
            bool image_ok;
            auto info = Filesystem::GetObjectInfo(out_image_file, &image_ok);
            if (image_ok)
            {
                if (info.category != Filesystem::file)
                    Program::Error("Texture atlas image `", out_image_file, "` is not a file.");
                image_time_modified = info.time_modified;
            }
        }

        std::time_t desc_time_modified = 0;

        { // Get description file modification time. 0 if no file.
            bool desc_ok;
            auto info = Filesystem::GetObjectInfo(out_desc_file, &desc_ok);
            if (desc_ok)
            {
                if (info.category != Filesystem::file)
                    Program::Error("Texture atlas description `", out_desc_file, "` is not a file.");
                desc_time_modified = info.time_modified;
            }
        }


        // Decide if we should load the atlas or regenerate it.
        if (!allow_regeneration || source_tree.time_modified_recursive < min(image_time_modified, desc_time_modified))
        {
            // Try loading the existing atlas because either regeneration is disabled, or atlas image and description are new enough.
            try
            {
                // Load and parse description.
                Refl::FromString(desc, Stream::Input(out_desc_file));

                // Make sure that all requested artifical regions are present in the atlas. If not, attempt to regenerate it.
                for (const auto &[name, size] : artifical_regions)
                {
                    auto it = desc.images.find(name);
                    if (it == desc.images.end() || it->second.size != size)
                        Program::Error("The texture atlas doesn't include some of the requested artifical regions.");
                }

                // Load image.
                image = Image(out_image_file);

                return; // The atlas was loaded successfully.
            }
            catch (...)
            {
                // Unable to load the atlas.
                // If regeneration is allowed, we swallow the exception and try to regenerate.
                // Otherwise the exception is propagated.
                if (!allow_regeneration)
                    throw;
            }
        }

        // Begin regenerating atlas.

        // Count images.
        int image_count = artifical_regions.size();
        Filesystem::ForEachObject(source_tree, [&](const Filesystem::TreeNode &node)
        {
            if (node.info.category != Filesystem::file)
                return;
            image_count++;
        });

        // Load images.
        struct Elem
        {
            std::string name;
            Image image;
        };
        std::vector<Elem> elem_list;
        elem_list.reserve(image_count);

        for (const auto &[name, size] : artifical_regions)
        {
            auto &new_elem = elem_list.emplace_back();
            new_elem.name = name;
            new_elem.image = Image(size);
        }

        Filesystem::ForEachObject(source_tree, [&](const Filesystem::TreeNode &node)
        {
            if (node.info.category != Filesystem::file)
                return;

            auto &new_elem = elem_list.emplace_back();

            // Save image name, but first strip source directory name from it.
            new_elem.name = node.path.substr(source_dir.size() + 1); // `+ 1` is for `/`.

            // Load image.
            new_elem.image = Image(node.path);
        });

        // Sort images by name. Otherwise the order sometimes turns out different on different platforms.
        std::sort(elem_list.begin(), elem_list.end(), [](const Elem &a, const Elem &b){return a.name < b.name;});

        // Construct rectangle list for packing.
        std::vector<Packing::Rect> rect_list;
        rect_list.reserve(image_count);
        for (const Elem &elem : elem_list)
            rect_list.push_back(elem.image.Size());

        // Try packing rectangles.
        if (Packing::PackRects(target_size, rect_list.data(), rect_list.size(), add_gaps))
            Program::Error("Unable to fit texture atlas for `", source_dir, "` into a ", target_size.x, 'x', target_size.y, " texture.");

        // Construct description and final image.
        image = Image(target_size, u8vec4(0));
        desc = {}; // In case we started populating it and failed.
        for (size_t i = 0; i < elem_list.size(); i++)
        {
            // Add image to description.
            ImageDesc image_desc;
            image_desc.pos = rect_list[i].pos;
            image_desc.size = elem_list[i].image.Size(); // Note that we don't extract sizes from rectangles, since those sizes might include gap size.
            if (!desc.images.insert({std::move(elem_list[i].name), image_desc}).second)
                Program::Error("Internal error while generating description for texture atlas for `", source_dir, "`: Duplicate image paths.");

            // Copy this image to target image.
            image.UnsafeDrawImage(elem_list[i].image, image_desc.pos);
        }

        // Save final image.
        try
        {
            image.Save(out_image_file);
        }
        catch (...) {}

        // Save description.
        try
        {
            std::string desc_string = Refl::ToString(desc, Refl::ToStringOptions::Pretty());
            Stream::SaveFile(out_desc_file, desc_string, Stream::text);
        }
        catch (...) {}
    }
}
