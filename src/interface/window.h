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

    [[nodiscard]] inline ivec2 PosUndefined() {return ivec2(SDL_WINDOWPOS_UNDEFINED);}
    [[nodiscard]] inline ivec2 PosCentered() {return ivec2(SDL_WINDOWPOS_CENTERED);}

    struct WindowSettings
    {
        WindowSettings() {}

        // Returns a short readable summary of window settings. Some less important settings are not included.
        [[nodiscard]] std::string Summary() const;

        ivec2 pos = PosCentered();
        ivec2 min_size = ivec2(0);
        bool fixed_size = false;
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

        bool gl_debug = false;
        VSync vsync = VSync::adaptive;
        YesOrNo hw_accelerated = dont_care;
        bool fwd_compat = false;
        int msaa = 0; // 0 and 1 mean the same thing, no antialiasing.
        ivec4 color_bits = ivec4(0); // 0 mean "unspecified".
        int depth_bits = 0, stencil_bits = 0; // Same.
    };

    // A window class.
    // It's reference-counted, but only one actual window can exist at a time.
    class Window
    {
        struct Data;

        // Once we decide to support several windows, this should be replaced by something else.
        inline static std::weak_ptr<Data> global_data;

        std::shared_ptr<Data> data;

      public:
        Window() {}
        Window(const Window &) = default;
        Window(Window &&) = default;
        Window &operator=(const Window &) = default;
        Window &operator=(Window &&) = default;
        ~Window();

        [[nodiscard]] explicit operator bool() const {return bool(data);}

        Window(std::string title, ivec2 size, FullscreenMode mode = windowed, const WindowSettings &settings = {});

        // Unlike `Get()`, this doesn't throw if there is no window.
        [[nodiscard]] static SDL_Window *GetHandleOrNull();

        [[nodiscard]] static bool IsOpen();

        // Will throw if the window doesn't exist.
        [[nodiscard]] static Window Get();

        [[nodiscard]] SDL_Window *Handle() const;
        [[nodiscard]] SDL_GLContext Context() const;

        void SetTitle(std::string new_title);
        [[nodiscard]] const std::string &Title() const;

        [[nodiscard]] ivec2 Size() const;
        [[nodiscard]] bool Resizable() const;

        // Sets VSync mode, automatically falling back to different modes on failure.
        // If it fails to set even a fallback mode, does nothing and reports `unspecified` mode.
        // Passing `unspecified` to this function makes the window forget the actual mode and
        // report `unspecified`, but is otherwise a no-op.
        void SetVSyncMode(VSync new_vsync);
        [[nodiscard]] VSync VSyncMode() const;

        // If the window is not resizable, then `borderless_fullscreen` (which would require a window resize) acts as `fullscreen`.
        void SetMode(FullscreenMode new_mode);
        [[nodiscard]] FullscreenMode Mode() const;

        // Processes events, increments the tick counter.
        // If hooks are specified, applies them in order to each event. If a hook returns false, the current event is discarded.
        void ProcessEvents(const std::vector<std::function<bool(SDL_Event &)>> &hooks = {});

        // Updates the picture on the screen, increments the frame counter.
        void SwapBuffers();

        // Those counters initially return 1.
        // This lets us use 0 for events that never happened.
        [[nodiscard]] uint64_t Ticks() const;
        [[nodiscard]] uint64_t Frames() const;

        // Those return true for one tick after the corresponding event happend.
        // We make sure that a resize is always reported after `Tick()` is called for the first time.
        [[nodiscard]] bool Resized() const;
        [[nodiscard]] bool ExitRequested() const;

        // Returns true if the window is focused.
        [[nodiscard]] bool HasKeyboardFocus() const;
        // Returns true if the window is hovered.
        [[nodiscard]] bool HasMouseFocus() const; // Returns 1 if the window is hovered.

        // Returns text that was entered during the last tick, in UTF-8.
        [[nodiscard]] const std::string &TextInput() const;

        struct InputTimes
        {
            uint64_t press = 0, release = 0, repeat = 0;
        };
        // Returns the information about a specific button.
        // The values represent the last time points when a specific action happened to the button.
        // They are taken from the `Ticks()` counter.
        [[nodiscard]] InputTimes GetInputTimes(Input::Enum index) const;

        // Returns mouse position.
        [[nodiscard]] ivec2 MousePos() const;
        // Returns the change in mouse position since the last tick.
        // Works even when the mouse is in the relative mode.
        [[nodiscard]] ivec2 MouseMovement() const;

        // Hides the mouse cursor while it hovers the window.
        void HideCursor(bool hide = true);
        // Hides the cursor and prevents the mouse from leaving the window.
        // `MousePos()` won't work properly in this mode.
        void RelativeMouseMode(bool relative = true);

        // Files dragged onto the window at the last tick.
        [[nodiscard]] const std::vector<std::string> &DroppedFiles();
        [[nodiscard]] const std::vector<std::string> &DroppedStrings(); // Text dragged onto the window at the last tick.
    };
}
