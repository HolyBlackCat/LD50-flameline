#pragma once

#include <memory>

#include "utils/mat.h"

namespace Graphics
{
    struct ShaderConfig;
    class FrameBuffer;
}

class AdaptiveViewport
{
    struct Data;
    std::unique_ptr<Data> data;

  public:
    class Details
    {
        ivec2 size = ivec2(0);
        float scale = 1;
        int scale_floor = 1;
        ivec2 intermediate_size = ivec2(0);
        ivec2 target_size = ivec2(0);
        ivec2 viewport_pos = ivec2(0), viewport_size = ivec2(0);

      public:
        Details() {}

        Details(ivec2 new_size)
        {
            SetSize(new_size);
        }

        void SetSize(ivec2 new_size) // Needs Update() to take effect. Before that the state is indeterminate.
        {
            size = new_size;
        }

        void Update(ivec2 new_target_size) // Recalculates viewport based on selected size and current window size.
        {
            target_size = new_target_size;
            scale = (fvec2(new_target_size) / fvec2(size)).min();
            scale_floor = max(1, int(scale));
            intermediate_size = size * scale_floor;
            viewport_size = min(iround(size * scale), new_target_size);
            viewport_pos = (new_target_size - viewport_size)/2;
        }

        ivec2 Size() const
        {
            return size;
        }
        ivec2 TargetSize() const // Normally this is a window size.
        {
            return target_size;
        }
        float Scale() const
        {
            return scale;
        }
        ivec2 IntermediateSize() const
        {
            return intermediate_size;
        }
        ivec2 ViewportPos() const
        {
            return viewport_pos;
        }
        ivec2 ViewportSize() const
        {
            return viewport_size;
        }

        fmat4 MatrixCentered() const // Doesn't depend on target size and works without Update().
        {
            ivec2 a = -size/2;
            ivec2 b = a + size;
            return fmat4::ortho(ivec2(a.x, b.y), ivec2(b.x, a.y), -1, 1);
        }
        fmat4 Matrix() const // Doesn't depend on target size and works without Update().
        {
            return fmat4::ortho(ivec2(0, size.y), ivec2(size.x, 0), -1, 1);
        }

        fmat3 MouseMatrixCentered() const
        {
            return fmat3::scale(fvec2(1 / scale)) * fmat3::translate(-ViewportPos() - ViewportSize()/2);
        }
        fmat3 MouseMatrix() const
        {
            return fmat3::scale(fvec2(1 / scale)) * fmat3::translate(-ViewportPos());
        }
    };

    AdaptiveViewport(decltype(nullptr));

    AdaptiveViewport(const Graphics::ShaderConfig &shader_config);
    AdaptiveViewport(const Graphics::ShaderConfig &shader_config, ivec2 new_size);

    AdaptiveViewport(AdaptiveViewport &&);
    AdaptiveViewport &operator=(AdaptiveViewport &&);
    ~AdaptiveViewport();

    void SetSize(ivec2 new_size); // Needs Update() to take effect. Before that the state is indeterminate.

    void Update(ivec2 new_target_size); // Recalculates viewport based on selected size and current window size.
    void Update(); // Same, but uses current window size.

    void BeginFrame(); // Binds the internal framebuffer and sets a proper viewport for it.
    void FinishFrame(const Graphics::FrameBuffer *fbuf = 0); // Rescales and outputs the frame to the passed framebuffer.

    const Details &GetDetails() const;
};
