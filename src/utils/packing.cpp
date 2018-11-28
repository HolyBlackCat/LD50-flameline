#include "packing.h"

#include <algorithm>
#include <memory>
#include <vector>

#include <stb_rect_pack.h>

namespace Packing
{
    int PackRects(ivec2 target_size, Rect *data, int count, int inner_gaps, int outer_gaps)
    {
        // Adjust size.
        target_size -= 2 * outer_gaps;
        target_size += inner_gaps;

        // Make rectangle vector.
        std::vector<stbrp_rect> rects(count);
        for (int i = 0; i < count; i++)
        {
            ivec2 rect_size = data[i].size + inner_gaps;
            rects[i].w = rect_size.x;
            rects[i].h = rect_size.y;
        }

        // Allocate a buffer.
        // Comments on `stb_rect_pack` say we need the amount of elements equal to target width.
        int buffer_size = target_size.x;
        auto packing_buffer = std::make_unique<stbrp_node[]>(buffer_size);

        // Make a context.
        stbrp_context context;
        stbrp_init_target(&context, target_size.x, target_size.y, packing_buffer.get(), buffer_size); // No cleanup is needed.

        // Try packing.
        bool ok = stbrp_pack_rects(&context, rects.data(), rects.size());

        // Counts rectangles that weren't packed.
        int rects_not_packed = 0;
        if (!ok)
            std::count_if(rects.begin(), rects.end(), [](const stbrp_rect &rect){return rect.was_packed;});

        // Output data.
        for (int i = 0; i < count; i++)
        {
            data[i].pos = ivec2(rects[i].x, rects[i].y) + outer_gaps;
            data[i].was_packed = rects[i].was_packed;
        }

        return rects_not_packed;
    }
}
