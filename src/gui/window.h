#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <memory>
#include <string>

#include <SDL2/SDL.h>

#include "mat.h"

namespace GUI
{
    namespace Inputs
    {
        enum Enum
        {
            // No input
            None = 0,

            // Keys
            BeginKeys = 0, // It's guaranteed that key 0 is never used.

            _0 = SDL_SCANCODE_0,
            _1 = SDL_SCANCODE_1,
            _2 = SDL_SCANCODE_2,
            _3 = SDL_SCANCODE_3,
            _4 = SDL_SCANCODE_4,
            _5 = SDL_SCANCODE_5,
            _6 = SDL_SCANCODE_6,
            _7 = SDL_SCANCODE_7,
            _8 = SDL_SCANCODE_8,
            _9 = SDL_SCANCODE_9,

            a = SDL_SCANCODE_A,
            b = SDL_SCANCODE_B,
            c = SDL_SCANCODE_C,
            d = SDL_SCANCODE_D,
            e = SDL_SCANCODE_E,
            f = SDL_SCANCODE_F,
            g = SDL_SCANCODE_G,
            h = SDL_SCANCODE_H,
            i = SDL_SCANCODE_I,
            j = SDL_SCANCODE_J,
            k = SDL_SCANCODE_K,
            l = SDL_SCANCODE_L,
            m = SDL_SCANCODE_M,
            n = SDL_SCANCODE_N,
            o = SDL_SCANCODE_O,
            p = SDL_SCANCODE_P,
            q = SDL_SCANCODE_Q,
            r = SDL_SCANCODE_R,
            s = SDL_SCANCODE_S,
            t = SDL_SCANCODE_T,
            u = SDL_SCANCODE_U,
            v = SDL_SCANCODE_V,
            w = SDL_SCANCODE_W,
            x = SDL_SCANCODE_X,
            y = SDL_SCANCODE_Y,
            z = SDL_SCANCODE_Z,

            apostrophe = SDL_SCANCODE_APOSTROPHE,
            comma      = SDL_SCANCODE_COMMA,
            period     = SDL_SCANCODE_PERIOD,
            minus      = SDL_SCANCODE_MINUS,
            equals     = SDL_SCANCODE_EQUALS,
            semicolon  = SDL_SCANCODE_SEMICOLON,
            slash      = SDL_SCANCODE_SLASH,
            l_bracket  = SDL_SCANCODE_LEFTBRACKET,
            r_bracket  = SDL_SCANCODE_RIGHTBRACKET,
            backslash  = SDL_SCANCODE_BACKSLASH,
            grave      = SDL_SCANCODE_GRAVE,

            num_0 = SDL_SCANCODE_KP_0,
            num_1 = SDL_SCANCODE_KP_1,
            num_2 = SDL_SCANCODE_KP_2,
            num_3 = SDL_SCANCODE_KP_3,
            num_4 = SDL_SCANCODE_KP_4,
            num_5 = SDL_SCANCODE_KP_5,
            num_6 = SDL_SCANCODE_KP_6,
            num_7 = SDL_SCANCODE_KP_7,
            num_8 = SDL_SCANCODE_KP_8,
            num_9 = SDL_SCANCODE_KP_9,

            num_period    = SDL_SCANCODE_KP_PERIOD,
            num_equals    = SDL_SCANCODE_KP_EQUALS,
            num_enter     = SDL_SCANCODE_KP_ENTER,
            num_plus      = SDL_SCANCODE_KP_PLUS,
            num_minus     = SDL_SCANCODE_KP_MINUS,
            num_multiply  = SDL_SCANCODE_KP_MULTIPLY,
            num_divide    = SDL_SCANCODE_KP_DIVIDE,

            f1  = SDL_SCANCODE_F1,
            f2  = SDL_SCANCODE_F2,
            f3  = SDL_SCANCODE_F3,
            f4  = SDL_SCANCODE_F4,
            f5  = SDL_SCANCODE_F5,
            f6  = SDL_SCANCODE_F6,
            f7  = SDL_SCANCODE_F7,
            f8  = SDL_SCANCODE_F8,
            f9  = SDL_SCANCODE_F9,
            f10 = SDL_SCANCODE_F10,
            f11 = SDL_SCANCODE_F11,
            f12 = SDL_SCANCODE_F12,

            menu         = SDL_SCANCODE_MENU,
            escape       = SDL_SCANCODE_ESCAPE,
            enter        = SDL_SCANCODE_RETURN,
            tab          = SDL_SCANCODE_TAB,
            backspace    = SDL_SCANCODE_BACKSPACE,
            insert       = SDL_SCANCODE_INSERT,
            del          = SDL_SCANCODE_DELETE,
            up           = SDL_SCANCODE_UP,
            down         = SDL_SCANCODE_DOWN,
            left         = SDL_SCANCODE_LEFT,
            right        = SDL_SCANCODE_RIGHT,
            page_up      = SDL_SCANCODE_PAGEUP,
            page_down    = SDL_SCANCODE_PAGEDOWN,
            home         = SDL_SCANCODE_HOME,
            end          = SDL_SCANCODE_END,
            caps_lock    = SDL_SCANCODE_CAPSLOCK,
            scroll_lock  = SDL_SCANCODE_SCROLLLOCK,
            num_lock     = SDL_SCANCODE_NUMLOCKCLEAR,
            print_screen = SDL_SCANCODE_PRINTSCREEN,
            pause        = SDL_SCANCODE_PAUSE,
            space        = SDL_SCANCODE_SPACE,

            l_shift      = SDL_SCANCODE_LSHIFT,
            r_shift      = SDL_SCANCODE_RSHIFT,
            l_ctrl       = SDL_SCANCODE_LCTRL,
            r_ctrl       = SDL_SCANCODE_RCTRL,
            l_alt        = SDL_SCANCODE_LALT,
            r_alt        = SDL_SCANCODE_RALT,
            l_gui        = SDL_SCANCODE_LGUI,
            r_gui        = SDL_SCANCODE_RGUI,

            EndKeys = SDL_NUM_SCANCODES,

            // Mouse buttons
            BeginMouseButtons = EndKeys,

            mouse_left = BeginMouseButtons,
            mouse_middle,
            mouse_right,
            mouse_x1,
            mouse_x2,

            EndMouseButtons = BeginMouseButtons + 32,

            // Mouse wheen
            BeginMouseWheel = EndMouseButtons,

            mouse_wheel_up = BeginMouseWheel,
            mouse_wheel_down,
            mouse_wheel_left,
            mouse_wheel_right,

            EndMouseWheel = BeginMouseWheel + 4,

            // Index count
            IndexCount = EndMouseWheel,
        };
    }

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
        struct Data;
        std::unique_ptr<Data> data;

        inline static Window *instance = 0;

      public:
        Window();
        Window(Window &&other) noexcept;
        Window &operator=(Window &&other) noexcept;
        ~Window();

        Window(std::string name, ivec2 size, FullscreenMode mode = windowed, const Settings &settings = {});

        static Window &Instance(); // This will throw if there is no window.

        SDL_Window *Handle() const;
        SDL_GLContext Context() const;

        ivec2 Size() const;

        VSync VSyncMode() const;
        bool Resizable() const;

        void SetMode(FullscreenMode new_mode); // If the window is not resizable, then `borderless_fullscreen` (which requires a window resize) acts as `fullscreen`.
        FullscreenMode Mode() const;

        void Tick();
        void Swap();

        // Those counters start from 1.
        uint64_t Ticks() const;
        uint64_t Frames() const;

        // Those return 1 for one tick after the corresponding event happend.
        bool Resized() const;
        bool ExitRequested() const;

        bool HasKeyboardFocus() const; // Returns 1 if the window active.
        bool HasMouseFocus() const; // Returns 1 if the window if hovered.

        std::string TextInput() const;

        struct Input
        {
            uint64_t press = 0, release = 0, repeat = 0;
        };
        Input InputTimes(Inputs::Enum index) const;

        ivec2 MousePos() const;
        ivec2 MousePosDelta() const;

        void HideCursor(bool hide = 1);
        void RelativeMouseMode(bool relative = 1);
    };
}

#endif
