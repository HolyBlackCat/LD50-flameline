#pragma once

#include <unordered_map>

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

        int ascent = 0;
        int descent = 0;
        int line_skip = 0;

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

        Glyph &DefaultGlyph()
        {
            return default_glyph;
        }
        const Glyph &DefaultGlyph() const
        {
            return default_glyph;
        }

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
