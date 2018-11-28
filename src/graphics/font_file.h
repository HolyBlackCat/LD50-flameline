#pragma once

#include <algorithm>
#include <functional>
#include <exception>
#include <utility>

#include <ft2build.h>
#include FT_FREETYPE_H // Ugh.

#include "graphics/font.h"
#include "graphics/image.h"
#include "program/errors.h"
#include "utils/finally.h"
#include "utils/mat.h"
#include "utils/memory_file.h"
#include "utils/range_set.h"
#include "utils/strings.h"


namespace Graphics
{
    // Unlike most other graphics classes, `FontFile` doesn't rely on SDL or OpenGL. You can safely call it even if they aren't initialized.

    class FontFile
    {
        inline static bool ft_initialized = 0;
        inline static FT_Library ft_context = 0;
        inline static int open_font_count = 0;

        struct Data
        {
            FT_Face ft_font = 0;
            MemoryFile file;
        };

        Data data;

      public:
        FontFile(decltype(nullptr)) {}

        // File is copied into the font, since FreeType requires original data to be available when the font is used. (Since files are ref-counted, file contents aren't copied.)
        // `size` is measured in pixels. Normally you only provide height, but you can also provide width. In this case, `[x,0]` and `[0,x]` are equivalent to `[x,x]` due to how FreeType operates.
        // Some font files contain several fonts; `index` determines which one of them is loaded. Upper 16 bits of `index` contain so-called "variation" (sub-font?) index, which starts from 1. Use 0 to load the default one.

        FontFile(MemoryFile file, int size, int index = 0) : FontFile(file, ivec2(0, size), index) {}

        FontFile(MemoryFile file, ivec2 size, int index = 0)
        {
            if (!ft_initialized)
            {
                ft_initialized = !FT_Init_FreeType(&ft_context);
                if (!ft_initialized)
                    Program::Error("Unable to initialize FreeType.");
                // We don't unload the library if this constructor throws after this point.
            }

            data.file = std::move(file); // Memofy files are ref-counted, but moving won't hurt.

            FT_Open_Args args{};
            args.flags = FT_OPEN_MEMORY;
            args.memory_base = data.file.data();
            args.memory_size = data.file.size();

            if (FT_Open_Face(ft_context, &args, index, &data.ft_font))
                Program::Error("Unable to load font `", data.file.name(), "`.");
            FINALLY_ON_THROW( FT_Done_Face(data.ft_font); )

            if (FT_Set_Pixel_Sizes(data.ft_font, size.x, size.y))
            {
                // This size is not available. Out of courtesy we print a list of available sizes.

                // Convert the requested size to printable format.
                if (size.y == 0)
                    size.x = size.y;
                else if (size.x == 0)
                    size.y = size.x;
                std::string requested_size;
                if (size.x == size.y)
                    requested_size = Str(size.y);
                else
                    requested_size = Str(size.x, 'x', size.y);

                // Get the list of available sizes.
                std::string size_list;
                for (int i = 0; i < int(data.ft_font->num_fixed_sizes); i++)
                {
                    if (i != 0)
                        size_list += ", ";

                    ivec2 this_size(data.ft_font->available_sizes[i].width, data.ft_font->available_sizes[i].height);

                    if (this_size.x == this_size.y)
                        size_list += Str(this_size.y);
                    else
                        size_list += Str(this_size.x, 'x', this_size.y);
                }

                Program::Error("Bitmap font ", data.file.name(), "`", index != 0 ? Str("[",index,"]") : "", " doesn't support size ", requested_size, ".",
                               size_list.empty() ? "" : Str("\nAvailable sizes are: ", size_list, "."));
            }

            open_font_count++; // This must remain at the bottom of the constructor in case something throws.
        }

        FontFile(FontFile &&other) noexcept : data(std::exchange(other.data, {})) {}
        FontFile &operator=(FontFile other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        ~FontFile()
        {
            if (data.ft_font)
            {
                FT_Done_Face(data.ft_font);
                open_font_count--;
            }
        }

        static void UnloadLibrary() // Use this to unload freetype. This function throws if you have opened fonts. When
        {
            if (open_font_count > 0)
                Program::Error("Unable to unload FreeType: ", open_font_count, " fonts are still in use.");
            if (ft_initialized)
                return;
            FT_Done_FreeType(ft_context);
            ft_initialized = 0;
        }

        explicit operator bool() const
        {
            return bool(data.ft_font);
        }

        int Ascent() const
        {
            return data.ft_font->size->metrics.ascender >> 6; // Ascent is stored as 26.6 fixed point and it's supposed to be already rounded, so we truncate it.
        }
        int Descent() const
        {
            return -(data.ft_font->size->metrics.descender >> 6); // See `Ascent()` for why we bit-shift. Note that we also have to negate descent to make it positive.
        }
        int Height() const
        {
            return Ascent() + Descent();
        }
        int LineSkip() const
        {
            return data.ft_font->size->metrics.height >> 6; // Huh, freetype calls it 'height'. See `Ascent()` for why we bit-shift.
        }
        int LineGap() const
        {
            return LineSkip() - Height();
        }
        bool HasKerning() const
        {
            return FT_HAS_KERNING(data.ft_font);
        }
        int Kerning(uint32_t a, uint32_t b) const // Returns 0 if there is no kerning for this font. Add the returned value to horizontal offset between two glyphs.
        {
            if (!HasKerning())
                return 0;
            FT_Vector vec;
            if (FT_Get_Kerning(data.ft_font, FT_Get_Char_Index(data.ft_font, a), FT_Get_Char_Index(data.ft_font, b), FT_KERNING_DEFAULT, &vec))
                return 0;
            return (vec.x + (1 << 5)) >> 6; // The kerning is measured in 26.6 fixed point pixels, so we round it.
        }

        // Constructs a functor to return kerning. Freetype font handle is copied into the functior, you should keep corresponding object alive as long as you need it.
        // If the font doesn't support kerning, null functor is returned.
        std::function<int(uint32_t, uint32_t)> KerningFunc() const
        {
            if (!HasKerning())
                return 0;
            return [ft_font = data.ft_font](uint16_t a, uint16_t b) -> int
            {
                FT_Vector vec;
                if (FT_Get_Kerning(ft_font, FT_Get_Char_Index(ft_font, a), FT_Get_Char_Index(ft_font, b), FT_KERNING_DEFAULT, &vec))
                    return 0;
                return (vec.x + (1 << 5)) >> 6; // See `Kerning()` for why we bit-shift.
            };
        }

        bool HasChar(uint32_t ch) const
        {
            return bool(FT_Get_Char_Index(data.ft_font, ch));
        }

        enum RenderMode
        {
            normal = FT_LOAD_TARGET_NORMAL,
            light  = FT_LOAD_TARGET_LIGHT,
            mono   = FT_LOAD_TARGET_MONO, // `mono` means "monochrome", not "monospaced".
        };

        struct GlyphData
        {
            Image image;
            ivec2 offset;
            int advance;
        };

        GlyphData GetChar(uint32_t ch, RenderMode mode) const
        {
            try
            {
                if (!HasChar(ch))
                    Program::Error("No such glyph.");
                if (FT_Load_Char(data.ft_font, ch, FT_LOAD_RENDER | mode))
                    Program::Error("Unknown error.");

                auto glyph = data.ft_font->glyph;
                auto bitmap = glyph->bitmap;

                bool is_antialiased;
                switch (bitmap.pixel_mode)
                {
                  case FT_PIXEL_MODE_MONO:
                    is_antialiased = 1;
                    break;
                  case FT_PIXEL_MODE_GRAY:
                    is_antialiased = 0;
                    break;
                  default:
                    Program::Error("Bitmap format ", bitmap.pixel_mode, " is not supported.");
                    break;
                }

                GlyphData ret;
                ivec2 size = ivec2(bitmap.width, bitmap.rows);
                ret.offset = ivec2(glyph->bitmap_left, -glyph->bitmap_top);
                ret.advance = (glyph->advance.x + (1 << 5)) >> 6; // Advance is measured in 26.6 fixed point pixels, so we round it.
                ret.image = Image(size);

                if (is_antialiased)
                {
                    for (int y = 0; y < size.y; y++)
                        std::copy(bitmap.buffer + bitmap.pitch * y, bitmap.buffer + bitmap.pitch * y + size.x, (uint8_t *)ret.image.Data() + size.x * y);
                }
                else
                {
                    for (int y = 0; y < size.y; y++)
                    {
                        uint8_t *byte_ptr = bitmap.buffer + bitmap.pitch * y;
                        uint8_t byte;
                        for (int x = 0; x < size.x; x++)
                        {
                            if (x % 8 == 0)
                                byte = *byte_ptr++;
                            ret.image.UnsafeAt(ivec2(x,y)) = u8vec4(byte & 128 ? 255 : 0);
                            byte <<= 1;
                        }
                    }
                }

                return ret;
            }
            catch (std::exception &e)
            {
                Program::Error("Unable to render glyph ", ch, " for font `", data.file.name(), "`: ", e.what());
            }
        }
    };

    struct FontAtlasEntry
    {
        Font *target;
        const RangeSet<uint32_t> *glyphs;
        const FontFile *source;
    };
}
