#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <exception>
#include <string>
#include <utility>

#include <SDL2/SDL.h>

#include "mat.h"

namespace GUI
{
    class Window
    {
        // Only one instance can exist at a time.
      public:
        enum FullscreenMode {windowed, fullscreen, borderless_fullscreen};
        enum class VSync {enabled, disabled, adaptive, unspecified};

        enum class Profile {core, compatibility, es, any_profile};
        enum class Position {centered, undefined, custom};
        enum YesOrNo {yes, no, dont_care};

        struct Settings
        {
            ivec2 coords = ivec2(0);
            ivec2 min_size = ivec2(0);
            bool resizable = 0;

            int display = 0;
            Position position = Position::undefined;
            int gl_major = 3, gl_minor = 3; // 0,0 = don't care.
            Profile gl_profile = Profile::compatibility;
            bool gl_debug = 0;
            VSync vsync_mode = VSync::adaptive;
            YesOrNo hardware_acceleration = dont_care;
            bool forward_compatibility = 0;
            int msaa = 0;
            ivec4 color_bits = ivec4(0);
            int depth_bits = 0, stencil_bits = 0;

            Settings() {}
            std::string GetSummary() const; // Returns a short readable summary of window settings. Settings that can't cause a window creation error aren't included.

            using ref = Settings &;

            ref MinSize(ivec2 sz)
            {
                min_size = sz;
                return *this;
            }
            ref Resizable(bool r = 1)
            {
                resizable = r;
                return *this;
            }
            ref Display(int n)
            {
                display = n;
                return *this;
            }
            ref Position(ivec2 p)
            {
                position = Position::custom;
                coords = p;
                return *this;
            }
            ref Position_Centered()
            {
                position = Position::centered;
                return *this;
            }
            ref Position_Undefined()
            {
                position = Position::undefined;
                return *this;
            }
            ref GlVersion(int maj, int min) // 0,0 = don't care.
            {
                gl_major = maj;
                gl_minor = min;
                return *this;
            }
            ref GlProfile(Profile p)
            {
                gl_profile = p;
                return *this;
            }
            ref GlDebug(bool d = 1)
            {
                gl_debug = d;
                return *this;
            }
            ref VSyncMode(VSync v)
            {
                vsync_mode = v;
                return *this;
            }
            ref HardwareAcceleration(YesOrNo s)
            {
                hardware_acceleration = s;
                return *this;
            }
            ref ForwardCompatibility(YesOrNo f)
            {
                forward_compatibility = f;
                return *this;
            }
            ref MSAA(int m)
            {
                msaa = m;
                return *this;
            }
            ref ColorBits(ivec4 b)
            {
                color_bits = b;
                return *this;
            }
            ref DepthBits(int b)
            {
                depth_bits = b;
                return *this;
            }
            ref StencilBits(int b)
            {
                stencil_bits = b;
                return *this;
            }
        };

      private:
        SDL_Window *handle = 0;
        SDL_GLContext context = 0;

        // When adding variables here, don't forget to add them to move constructor/assignment.
        ivec2 size = ivec2(0);
        VSync vsync = VSync::unspecified;
        bool resizable = 0;
        FullscreenMode mode = FullscreenMode::windowed;

        inline static Window *instance = 0;

        void Destroy();

      public:
        Window() {}
        Window(Window &&other) noexcept;
        Window &operator=(Window &&other) noexcept;

        Window(std::string name, ivec2 size, FullscreenMode mode = windowed, const Settings &settings = {});

        ~Window();

        static ivec2 Size();

        static SDL_Window *Handle();
        static SDL_GLContext Context();

        static VSync VSyncMode();
        static bool Resizable();

        static void SetMode(FullscreenMode new_mode); // If the window is not resizable, then `borderless_fullscreen` (which requires a window resize) acts as `fullscreen`.
        static FullscreenMode Mode();

        static void Swap();
    };
}

#endif
