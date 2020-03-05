#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <cglfl/cglfl.hpp>
#include <SDL.h>

#include "input/enum.h"
#include "utils/mat.h"

namespace Interface
{
    enum FullscreenMode {windowed, fullscreen, borderless_fullscreen};
    enum class VSync {enabled, disabled, adaptive, unspecified};

    enum class Profile {core, compat, es, any};
    enum YesOrNo {yes, no, dont_care};

    inline ivec2 PosUndefined()
    {
        return ivec2(SDL_WINDOWPOS_UNDEFINED);
    }
    inline ivec2 PosCentered()
    {
        return ivec2(SDL_WINDOWPOS_CENTERED);
    }

    struct WindowSettings
    {
        ivec2 pos = PosCentered();
        ivec2 min_size = ivec2(0);
        bool fixed_size = 0;
        int display = 0;

        int gl_major = CGLFL_GL_MAJOR, gl_minor = CGLFL_GL_MINOR; // 0,0 = don't care.

        Profile gl_profile =
        #if defined(CGLFL_GL_API_gles)
            Profile::es;
        #elif defined(CGLFL_GL_PROFILE_core)
            Profile::core;
        #elif defined(CGLFL_GL_PROFILE_compat)
            Profile::compat;
        #else
            Profile::any;
        #endif

        bool gl_debug = 0;
        VSync vsync = VSync::adaptive;
        YesOrNo hw_accelerated = dont_care;
        bool fwd_compat = 0;
        int msaa = 0;
        ivec4 color_bits = ivec4(0);
        int depth_bits = 0, stencil_bits = 0;

        WindowSettings() {}

        std::string Summary() const; // Returns a short readable summary of window settings. Settings that can't cause a window creation error aren't included.
    };

    class Window
    {
        // Only one instance can exist at a time.
      public:

        struct InputTimes
        {
            uint64_t press = 0, release = 0, repeat = 0;
        };

      private:
        inline static Window *instance = 0;

        struct Data
        {
            SDL_Window *handle = 0;
            SDL_GLContext context = 0;

            std::string title;

            ivec2 size = ivec2(0);
            VSync vsync = VSync::unspecified;
            bool resizable = 0;
            FullscreenMode mode = FullscreenMode::windowed;

            uint64_t tick_counter = 1, frame_counter = 1;

            uint64_t resize_time = 0;
            uint64_t exit_request_time = 0;

            std::string text_input;

            ivec2 mouse_pos = ivec2(0);
            ivec2 mouse_movement = ivec2(0);

            bool keyboard_focus = 0, mouse_focus = 0;

            std::vector<InputTimes> input_times;

            std::vector<std::string> dropped_files, dropped_strings;
        };

        Data data;

      public:
        Window();
        Window(Window &&other) noexcept;
        Window &operator=(Window other) noexcept;
        ~Window();

        Window(std::string title, ivec2 size, FullscreenMode mode = windowed, const WindowSettings &settings = {});

        static SDL_Window *GetHandleOrNull(); // Unlike `Get()`, this doesn't throw if there is no window.

        static bool IsOpen();
        static Window &Get(); // This will throw if there is no window.

        SDL_Window *Handle() const;
        SDL_GLContext Context() const;

        void SetTitle(std::string new_title);
        std::string Title() const;

        ivec2 Size() const;

        VSync VSyncMode() const;
        bool Resizable() const;

        void SetMode(FullscreenMode new_mode); // If the window is not resizable, then `borderless_fullscreen` (which requires a window resize) acts as `fullscreen`.
        FullscreenMode Mode() const;

        void ProcessEvents(std::vector<std::function<bool(SDL_Event &)>> hooks = {}); // If a hook returns `false`, the current event is discarded.
        void SwapBuffers();

        // Those counters start from 1.
        uint64_t Ticks() const;
        uint64_t Frames() const;

        // Those return 1 for one tick after the corresponding event happend.
        bool Resized() const;
        bool ExitRequested() const;

        bool HasKeyboardFocus() const; // Returns 1 if the window is active.
        bool HasMouseFocus() const; // Returns 1 if the window is hovered.

        std::string TextInput() const;

        InputTimes GetInputTimes(Input::Enum index) const;

        ivec2 MousePos() const;
        ivec2 MouseMovement() const;

        void HideCursor(bool hide = 1);
        void RelativeMouseMode(bool relative = 1);

        const std::vector<std::string> &DroppedFiles(); // Files dragged onto the window at the last tick.
        const std::vector<std::string> &DroppedStrings(); // Text dragged onto the window at the last tick.
    };
}
