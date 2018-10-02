#include "texture_atlas.h"

#include <iostream>
#include <memory>

#include <stb_rect_pack.h>

TextureAtlas::TextureAtlas(ivec2 target_size, const std::string &source_dir, const std::string &out_image_file, const std::string &out_desc_file, bool add_gaps)
    : source_dir(source_dir)
{
    // Decide if regenrating the atlas should be allowed.
    bool allow_regeneration = source_dir.size() > 0;

    Filesystem::TreeNode source_tree;

    // If regeneration is allowed, get source directory tree and its modification time.
    if (allow_regeneration)
    {
        // Get the tree.
        try
        {
            Filesystem::TreeNode new_source_tree = Filesystem::GetObjectTree(source_dir); // This throws if no such file or directory.
            if (new_source_tree.info.category != Filesystem::directory)
                Program::Error("Texture atlas source location `", source_dir, "` is not a directory.");
            source_tree = std::move(new_source_tree); // We use a temporary to make sure that if it's not a directory, we have an empty file tree.
        }
        catch (...) {}
    }

    // Get image file modification time. 0 if no file.
    std::time_t image_time_modified = 0;
    try
    {
        auto info = Filesystem::GetObjectInfo(out_image_file);
        if (info.category != Filesystem::file)
            Program::Error("Texture atlas image `", out_image_file, "` is not a file.");
        image_time_modified = info.time_modified;
    }
    catch (...) {}

    // Get description file modification time. 0 if no file.
    std::time_t desc_time_modified = 0;
    try
    {
        auto info = Filesystem::GetObjectInfo(out_desc_file);
        if (info.category != Filesystem::file)
            Program::Error("Texture atlas description `", out_desc_file, "` is not a file.");
        desc_time_modified = info.time_modified;
    }
    catch (...) {}

    // Decide if we should load the atlas or regenerate it.
    if (!allow_regeneration || source_tree.time_modified_recursive < min(image_time_modified, desc_time_modified))
    {
        // Try loading the existing atlas because either regeneration is disabled, or atlas image and description are new enough.
        try
        {
            // Load image.
            image = Graphics::Image(out_image_file);

            // Load description.
            std::string desc_string = MemoryFile(out_desc_file).construct_string();

            // Parse description.
            Desc new_desc; // We use a temporary instead of `description` because if conversion fails, we might need `description` to be empty to regenerate the atlas into it.
            Refl::Interface(new_desc).from_string(desc_string.c_str());
            desc = std::move(new_desc);

            return; // The atlas is loaded successfully.
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
    int image_count = 0;
    Filesystem::ForEachObject(source_tree, [&](const Filesystem::TreeNode &node)
    {
        if (node.info.category != Filesystem::file)
            return;
        image_count++;
    });

    // Load images and construct rectangle list for packing.
    std::vector<std::string> name_list;
    name_list.reserve(image_count);

    std::vector<Graphics::Image> image_list;
    image_list.reserve(image_count);

    std::vector<stbrp_rect> rect_list;
    rect_list.reserve(image_count);

    Filesystem::ForEachObject(source_tree, [&](const Filesystem::TreeNode &node)
    {
        if (node.info.category != Filesystem::file)
            return;

        // Save image name, but first strip source director name from it.
        name_list.push_back(node.path.substr(source_dir.size() + 1)); // `+ 1` is for `/`.

        // Load image.
        image_list.push_back(Graphics::Image(node.path));

        // Make rectangle.
        auto &rect = rect_list.emplace_back();
        rect.w = image_list.back().Size().x;
        rect.h = image_list.back().Size().y;

        if (add_gaps)
        {
            rect.w++;
            rect.h++;
        }
    });

    // Prepare for packing rectangles.
    stbrp_context packing_context;
    ivec2 packing_size = target_size - (add_gaps ? 1 : 0);
    int packing_buffer_size = packing_size.x; // Comments in `stb_rect_pack` say we need the amount of elements equal to target width.
    auto packing_buffer = std::make_unique<stbrp_node[]>(packing_buffer_size);
    stbrp_init_target(&packing_context, packing_size.x, packing_size.y, packing_buffer.get(), packing_buffer_size); // No cleanup needed.

    // Try packing rectangles.
    if (!stbrp_pack_rects(&packing_context, rect_list.data(), rect_list.size()))
        Program::Error("Unable to fit texture atlas for `", source_dir, "` into ", target_size.x, 'x', target_size.y, " texture.");

    // Construct description and final image.
    image = Graphics::Image(target_size, u8vec4(0));
    for (size_t i = 0; i < image_list.size(); i++)
    {
        // Add image to description.
        ImageDesc image_desc;
        image_desc.pos = ivec2(rect_list[i].x, rect_list[i].y);
        image_desc.size = image_list[i].Size(); // Note that we don't extract sizes from rectangles, since those sizes might include gap size.
        if (!desc.images.insert({std::move(name_list[i]), image_desc}).second)
            Program::Error("Internal error while generating description for texture atlas for `", source_dir, "`: Duplicate image paths.");

        // Copy this image to target image.
        image.UnsafeDrawImage(image_list[i], image_desc.pos + (add_gaps ? 1 : 0));
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
        std::string desc_string = Refl::Interface(desc).to_string();
        MemoryFile::Save(out_desc_file, (uint8_t *)desc_string.data(), (uint8_t *)desc_string.data() + desc_string.size());
    }
    catch (...) {}
}
