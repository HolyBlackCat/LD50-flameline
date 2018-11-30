#pragma once

#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include "graphics/font.h"
#include "utils/mat.h"
#include "utils/unicode.h"

namespace Graphics
{
    struct Text
    {
        struct Symbol
        {
            uint32_t ch = 0;
            svec2 texture_pos = ivec2(0);
            svec2 offset = ivec2(0);
            svec2 size = ivec2(0);
            short advance = 0;
            short kerning = 0;

            short ascent = 0;
            short descent = 0;
            short line_gap = 0;
        };

        struct Line
        {
            std::vector<Symbol> symbols;
        };

        std::vector<Line> lines = {{}}; // We start with one line by default.


        struct Stats
        {
            struct Line
            {
                int width = 0;
                int ascent = 0;
                int descent = 0;
                int gap = 0;
            };
            std::vector<Line> lines;

            ivec2 size = ivec2(0);
        };
        Stats ComputeStats() const
        {
            Stats ret;

            for (const Line &line : lines)
            {
                Stats::Line &line_stats = ret.lines.emplace_back();

                line_stats.width = 0;

                constexpr int min_value = std::numeric_limits<int>::min();
                line_stats.ascent = min_value;
                line_stats.descent = min_value;
                line_stats.gap = min_value;

                for (const Symbol &symbol : line.symbols)
                {
                    line_stats.width += symbol.advance + symbol.kerning;
                    clamp_var_min(line_stats.ascent, symbol.ascent);
                    clamp_var_min(line_stats.descent, symbol.descent);
                    clamp_var_min(line_stats.gap, symbol.line_gap);
                }

                clamp_var_min(ret.size.x, line_stats.width);
                ret.size.y += line_stats.ascent + line_stats.descent + line_stats.gap;
            }

            ret.size.y -= ret.lines.back().gap;

            return ret;
        }


        Text() {}
        Text(const Font &font, const char *begin, const char *end = 0)
        {
            AddString(font, begin, end);
        }
        Text(const Font &font, std::string_view str)
        {
            AddString(font, str);
        }


        void AddSymbol(const Symbol &glyph)
        {
            if (glyph.ch == '\n')
                lines.emplace_back();
            else
                lines.back().symbols.push_back(glyph);
        }
        void AddSymbol(const Font &font, uint32_t ch)
        {
            if (ch == '\n')
            {
                lines.emplace_back();
            }
            else
            {
                const Font::Glyph &glyph = font.Get(ch);
                Symbol symbol;
                symbol.ch = ch;
                symbol.texture_pos = glyph.texture_pos;
                symbol.offset = glyph.offset;
                symbol.size = glyph.size;
                symbol.advance = glyph.advance;
                // We leave `.kerning` unchanged on purpose.
                symbol.ascent = font.Ascent();
                symbol.descent = font.Descent();
                symbol.line_gap = font.LineGap();
                lines.back().symbols.push_back(symbol);
            }
        }

        void KernLastTwoSymbols(const Font &font)
        {
            auto &symbols = lines.back().symbols;
            if (symbols.size() < 2)
                return;
            symbols[symbols.size() - 2].kerning = font.Kerning(symbols[symbols.size() - 2].ch, symbols[symbols.size() - 1].ch);
        }

        Text &AddString(const Font &font, const char *begin, const char *end = 0)
        {
            bool first = 1;
            for (uint32_t ch : Unicode::Iterator(begin, end))
            {
                if (first)
                    first = 0;
                else
                    KernLastTwoSymbols(font);

                AddSymbol(font, ch);
            }

            return *this;
        }
        Text &AddString(const Font &font, std::string_view str)
        {
            AddString(font, &*str.begin(), &*str.end());

            return *this;
        }
    };
}
