#pragma once

#include <memory>

#include "utils/mat.h"

namespace Graphics
{
    struct ShaderConfig;
    class FrameBuffer;
}

namespace GameUtils
{
    // Upscales pixel graphics in a pretty way.
    class AdaptiveViewport
    {
        struct Data;
        std::unique_ptr<Data> data;

      public:
        class Details
        {
            ivec2 size = ivec2(0); // Source size.
            float scale = 1; // How much the target is larger than the source.
            int scale_floor = 1; // `scale`, floored.
            ivec2 intermediate_size = ivec2(0); // The intermediate texture size, `size * scale_floor`.
            ivec2 target_size = ivec2(0); // Target size.
            ivec2 viewport_pos = ivec2(0); // Output viewport position.
            ivec2 viewport_size = ivec2(0); // Output viewport size. Same as `target_size`, but shrinked to proportion.

          public:
            Details() {}

            Details(ivec2 new_size)
            {
                SetSize(new_size);
            }

            // Sets the source size.
            // Needs Update() to take effect. Before that the state is indeterminate.
            void SetSize(ivec2 new_size)
            {
                size = new_size;
            }

            // Recalculates viewport based on previously selected size and `new_target_size` (normally the window size).
            void Update(ivec2 new_target_size)
            {
                target_size = new_target_size;
                scale = (fvec2(new_target_size) / fvec2(size)).min();
                scale_floor = max(1, int(scale));
                intermediate_size = size * scale_floor;
                viewport_size = min(iround(size * scale), new_target_size);
                viewport_pos = (new_target_size - viewport_size)/2;
            }

            // Source size.
            [[nodiscard]] ivec2 Size() const
            {
                return size;
            }
            // Target size. Normally this is the window size.
            [[nodiscard]] ivec2 TargetSize() const
            {
                return target_size;
            }
            // The scale factor, greater or equal to 1.
            [[nodiscard]] float Scale() const
            {
                return scale;
            }
            // The size of the intermediate texture, should be between `Size()` and `TargetSize()`.
            [[nodiscard]] ivec2 IntermediateSize() const
            {
                return intermediate_size;
            }
            // The resulting viewport position.
            [[nodiscard]] ivec2 ViewportPos() const
            {
                return viewport_pos;
            }
            // The resulting viewport size. Should match `TargetSize()`, but shrinked to proportion.
            [[nodiscard]] ivec2 ViewportSize() const
            {
                return viewport_size;
            }

            // Provides a matrix for rendering shaders. Puts the origin in the middle of the screen.
            // Doesn't depend on target size and works without `Update()`.
            [[nodiscard]] fmat4 MatrixCentered() const
            {
                ivec2 a = -size/2;
                ivec2 b = a + size;
                return fmat4::ortho(ivec2(a.x, b.y), ivec2(b.x, a.y), -1, 1);
            }
            // Provides a matrix for rendering shaders. Puts the origin in the corner.
            // Doesn't depend on target size and works without Update().
            [[nodiscard]] fmat4 Matrix() const
            {
                return fmat4::ortho(ivec2(0, size.y), ivec2(size.x, 0), -1, 1);
            }

            // The inverse matrix, to map mouse position to the source coordinate system. Puts the origin in the middle of the screen.
            [[nodiscard]] fmat3 MouseMatrixCentered() const
            {
                return fmat3::scale(fvec2(1 / scale)) * fmat3::translate(-ViewportPos() - ViewportSize()/2);
            }
            // The inverse matrix, to map mouse position to the source coordinate system. Puts the origin in the corner.
            [[nodiscard]] fmat3 MouseMatrix() const
            {
                return fmat3::scale(fvec2(1 / scale)) * fmat3::translate(-ViewportPos());
            }
        };

        AdaptiveViewport();

        // After this you need to call `SetSize` to set the source size.
        AdaptiveViewport(const Graphics::ShaderConfig &shader_config);

        // Sets the source size.
        AdaptiveViewport(const Graphics::ShaderConfig &shader_config, ivec2 new_size);

        AdaptiveViewport(AdaptiveViewport &&) noexcept;
        AdaptiveViewport &operator=(AdaptiveViewport &&) noexcept;
        ~AdaptiveViewport();

        // Sets the source size.
        // Needs Update() to take effect. Before that the state is indeterminate.
        void SetSize(ivec2 new_size);

        // Recalculates viewport based on previously selected size and `new_target_size` (normally the window size).
        void Update(ivec2 new_target_size);
        // Same, but uses current window size.
        void Update();

        // Binds the internal framebuffer and sets a proper viewport for it.
        void BeginFrame();
        // Rescales and outputs the frame to the passed framebuffer.
        void FinishFrame(const Graphics::FrameBuffer *fbuf = 0);

        // Returns some internal measurements.
        [[nodiscard]] const Details &GetDetails() const;
    };

}
