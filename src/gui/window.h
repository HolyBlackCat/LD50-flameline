#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <exception>
#include <string>

#include <SDL2/SDL.h>

#include "handle.h"
#include "mat.h"

namespace GUI
{
    class Window
    {
        // Only one instance can exist at a time.
      public:
        enum class VSync {enabled, disabled, adaptive, unspecified};
        enum class FullscreenMode {};

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
        Handle<SDL_Window *> handle;
        Handle<SDL_GLContext> context;

        VSync vsync = VSync::unspecified;

        inline static Window *instance = 0;

      public:
        Window() {}
        Window(std::string name, ivec2 size, const Settings &settings = {});
        ~Window();

        static SDL_Window *Handle();
        static SDL_GLContext Context();

        static VSync VSyncMode();
    };
}

#endif
