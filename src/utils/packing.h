#pragma once

#include "utils/mat.h"

namespace Packing
{
    struct Rect
    {
        // Input:
        ivec2 size = ivec2(0);

        // Output:
        ivec2 pos = ivec2(0);
        bool was_packed = 0;

        Rect() {}
        Rect(ivec2 size) : size(size) {}
    };

    // Returns 0 on success. On failure returns the amount of rectangles that didn't fit into the box.
    // Note that coordinates outside of [0;65535] range are not supported by default. This can be changed in `stb_rect_pack.h`.
    int PackRects(ivec2 target_size, Rect *data, int count, int inner_gaps = 0, int outer_gaps = 0);
}
