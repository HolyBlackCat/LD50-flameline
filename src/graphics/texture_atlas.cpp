#include "texture_atlas.h"

#include <memory>

#include <stb_rect_pack.h>

namespace Graphics
{
    // Pass empty string as `source_dir_name` to disallow regeneration.
    TextureAtlas::TextureAtlas(ivec2 target_size, const std::string &source_dir_name, const std::string &image_file_name, const std::string &desc_file_name, bool add_gaps)
    {
        image_name = image_file_name;

        // Decide if regenrating the atlas should be allowed.
        bool allow_regeneration = source_dir_name.size() > 0;

        Filesystem::TreeNode source_tree;
        std::time_t source_time_modified = 0;
        int image_count = 0;

        // If regeneration is allowed, get source directory tree and its modification time.
        if (allow_regeneration)
        {
            // Get the tree.
            try
            {
                Filesystem::TreeNode new_source_tree = Filesystem::GetEntryTree(source_dir_name); // This throws if no such file or directory.
                if (source_tree.info.category != Filesystem::directory)
                    Program::Error("Texture atlas source location `", source_dir_name, "` is not a directory.");
                source_tree = std::move(new_source_tree);
            }
            catch (...) {}

            // Get the latest modification time for all revelant files.
            Filesystem::ForEachFile(source_tree, [&](const Filesystem::TreeNode &node)
            {
                // Skip the file if it's not not revelant.
                if (!Strings::EndsWith(node.name, image_extension))
                    return;

                // Change modification time if needed.
                if (node.info.time_modified > source_time_modified)
                    source_time_modified = node.info.time_modified;

                // Increment image count.
                image_count++;
            });
        }

        // Get texture modification time. 0 if no texture.
        std::time_t tex_time_modified = 0;
        try
        {
            auto info = Filesystem::GetEntryInfo(image_file_name);
            if (info.category != Filesystem::file)
                Program::Error("Texture atlas image `", image_file_name, "` is not a file.");
            tex_time_modified = info.time_modified;
        }
        catch (...) {}

        // Get description modification time. 0 if no texture.
        std::time_t desc_time_modified = 0;
        try
        {
            auto info = Filesystem::GetEntryInfo(desc_file_name);
            if (info.category != Filesystem::file)
                Program::Error("Texture atlas description `", desc_file_name, "` is not a file.");
            desc_time_modified = info.time_modified;
        }
        catch (...) {}

        // Decide if we should load the atlas or regenerate it.
        if (!allow_regeneration || source_time_modified < min(tex_time_modified, desc_time_modified))
        {
            // Either regeneration is disabled, or atlas image and description are new enough. Here we try loading them instead.
            try
            {
                image = Image(image_file_name);

                std::string desc_string = MemoryFile(desc_file_name).construct_string();

                Desc new_desc; // We use a temporary instead of `description` because if conversion fails, we might need `description` to be empty to regenerate the atlas into it.
                Refl::Interface(new_desc).from_string(desc_string.c_str());
                description = std::move(new_desc);

                return; // The atlas is loaded successfully.
            }
            catch (...)
            {
                // We're unable to load the atlas.
                // If regeneration is allowed, we swallow the exception and try to regenerate. Otherwise the exception is propagated.
                if (!allow_regeneration)
                    throw;
            }
        }

        // We couldn't or decided not to load the existing atlas.
        // Here we try regenerating it.
        // At this point we're sure `allow_regeneration == 1`.

        // Load image list and construct rectangle list for packing. Also construct name list.
        std::vector<stbrp_rect> rect_list(image_count);
        std::vector<Image> image_list(image_count);
        std::vector<std::string> name_list(image_count);
        int index = 0;

        Filesystem::ForEachFile(source_tree, [&](const Filesystem::TreeNode &node)
        {
            // Skip the file if it's not not revelant.
            if (!Strings::EndsWith(node.name, image_extension))
                return;

            // Strip source directory name from path and save it.
            name_list[index] = node.path.substr(source_dir_name.size() + 1); // `+1` strips `/`.

            // Load image.
            auto &image = image_list[index];
            image = Image(node.path);

            // Make rectangle.
            auto &rect = rect_list[index];
            rect.w = image.Size().x;
            rect.h = image.Size().y;

            if (add_gaps)
            {
                rect.w++;
                rect.h++;
            }

            // Increment index.
            index++;
        });

        // Pack the rectangles.
        stbrp_context packing_context;
        ivec2 packing_size = (add_gaps ? target_size - 1 : target_size);
        int packing_buffer_size = packing_size.x; // Comments in `stb_rect_pack` say we need the amount of nodes equal to image width.
        auto packing_buffer = std::make_unique<stbrp_node[]>(packing_buffer_size);
        stbrp_init_target(&packing_context, packing_size.x, packing_size.y, packing_buffer.get(), packing_buffer_size); // No cleanup is needed.

        if (!stbrp_pack_rects(&packing_context, rect_list.data(), rect_list.size()))
            Program::Error("Unable to fit texture atlas from `", source_dir_name, "` into ", target_size.x, 'x', target_size.y, " texture.");

        // Construct description and final image.
        image = Image(target_size, u8vec4(0));
        for (index = 0; index < image_count; index++)
        {
            // Add image to description.
            ImageLocation new_image_desc;
            new_image_desc.pos = ivec2(rect_list[index].x, rect_list[index].y);
            new_image_desc.size = image_list[index].Size(); // Note that we don't extract sizes from rectangles, since those sizes might include gap size.
            if (!description.images.insert({std::move(name_list[index]), new_image_desc}).second)
                Program::Error("Internal error while generating description for texture atlas from `", source_dir_name, "`.");

            // Copy this image to target image.
            image.UnsafeDrawImage(image_list[index], add_gaps ? new_image_desc.pos + 1 : new_image_desc.pos);
        }

        // Save final image.
        try
        {
            image.Save(image_file_name);
        }
        catch (...) {}

        // Save description.
        try
        {
            std::string desc_string = Refl::Interface(description).to_string();
            MemoryFile::Save(desc_file_name, (uint8_t *)desc_string.data(), (uint8_t *)desc_string.data() + desc_string.size());
        }
        catch (...) {}
    }
}
