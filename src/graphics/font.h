#pragma once

#include <functional>
#include <unordered_map>
#include <utility>

#include "utils/mat.h"

namespace Graphics
{
    class Font
    {
      public:
        struct Glyph
        {
            ivec2 texture_pos = ivec2(0);
            ivec2 size = ivec2(0);
            ivec2 offset = ivec2(0);
            int advance = 0;
        };

      private:
        int ascent = 0;
        int descent = 0;
        int line_skip = 0;

        using kerning_func_t = std::function<int(uint32_t, uint32_t)>;
        kerning_func_t kerning_func = 0;

        // Some code might rely on references not being invalidated on insertion. Keep that in mind if you decide to change the container.
        std::unordered_map<uint32_t, Glyph> glyphs;
        Glyph default_glyph;

      public:
        void SetAscent(int new_ascent)
        {
            ascent = new_ascent;
        }
        void SetDescent(int new_descent)
        {
            descent = new_descent;
        }
        void SetLineSkip(int new_line_skip)
        {
            line_skip = new_line_skip;
        }
        void SetKerningFunc(kerning_func_t new_kerning_func) // You can use null function if you don't want kerning.
        {
            kerning_func = std::move(new_kerning_func);
        }

        int Ascent() const
        {
            return ascent;
        }
        int Descent() const
        {
            return descent;
        }
        int Height() const
        {
            return ascent + descent;
        }
        int LineSkip() const
        {
            return line_skip;
        }
        int LineGap() const
        {
            return line_skip - Height();
        }

        const kerning_func_t KerningFunc() const
        {
            return kerning_func;
        }
        bool HasKerning() const
        {
            return bool(kerning_func);
        }
        int Kerning(uint32_t a, uint32_t b) const
        {
            if (kerning_func)
                return kerning_func(a, b);
            else
                return 0;
        }

        Glyph &DefaultGlyph()
        {
            return default_glyph;
        }
        const Glyph &DefaultGlyph() const
        {
            return default_glyph;
        }

        // Note that returned references remain valid even after insertions.
        const Glyph &Get(uint32_t ch) const
        {
            if (auto it = glyphs.find(ch); it != glyphs.end())
                return it->second;
            else
                return default_glyph;
        }
        Glyph &Insert(uint32_t ch) // If the glyph already exists, returns a reference to it instead of creating a new one.
        {
            return glyphs.insert({ch, {}}).first->second;
        }
    };
}
