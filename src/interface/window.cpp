#include "window.h"

#include <cglfl/cglfl.hpp>

#include "macros/finally.h"
#include "program/errors.h"
#include "program/platform.h"
#include "strings/format.h"


// Export some variables to advise video drivers to use the best available video card for the application.
PLATFORM_IF(windows)
(
    extern "C"
    {
        __declspec(dllexport) uint32_t NvOptimusEnablement                  = 1; // For NVidia. Docs suggest that this should be of type DWORD, but I don't want windows headers here.
        __declspec(dllexport) int      AmdPowerXpressRequestHighPerformance = 1; // For AMD.
    }
)

namespace Interface
{
    std::string WindowSettings::Summary() const // Returns a short readable summary of window settings. Settings that can't cause a window creation error aren't included.
    {
        std::string ret;

        if (gl_major || gl_minor)
            ret = FMT("OpenGL {}.{}", gl_major, gl_minor);
        else
            ret = "Any OpenGL version";

        switch (gl_profile)
        {
            case Profile::core: ret += " (core)"; break;
            case Profile::compat: ret += " (compatibility)"; break;
            case Profile::es: ret += " ES"; break;
            case Profile::any: /* nothing */ break;
        }

        if (gl_debug)
            ret += ", debug mode";

        if (fwd_compat)
            ret += ", forward compatibility enabled";

        switch (hw_accelerated)
        {
            case yes: ret += ", hardware acceleration enabled"; break;
            case no: ret += ", hardware acceleration disabled"; break;
            case dont_care: /* nothing */ break;
        }

        if (msaa > 1)
            ret += FMT(", {}x MSAA", msaa);

        if (color_bits != 0 || depth_bits != 0 || stencil_bits != 0)
        {
            ret += ", bits per pixel:";
            if (color_bits.r != 0) ret += FMT(" r={}", color_bits.r);
            if (color_bits.g != 0) ret += FMT(" g={}", color_bits.g);
            if (color_bits.b != 0) ret += FMT(" b={}", color_bits.b);
            if (color_bits.a != 0) ret += FMT(" a={}", color_bits.a);
            if (depth_bits   != 0) ret += FMT(" depth={}", depth_bits);
            if (stencil_bits != 0) ret += FMT(" stencil={}", stencil_bits);
        }

        return ret;
    }

    struct Window::Data
    {
        SDL_Window *handle = nullptr;
        SDL_GLContext context = nullptr;

        std::string title;

        ivec2 size = ivec2(0);
        VSync vsync = VSync::unspecified;
        bool resizable = false;
        FullscreenMode mode = FullscreenMode::windowed;

        // Note that the counters start from 1.
        uint64_t tick_counter = 1, frame_counter = 1;

        uint64_t resize_time = 2; // Sic! This causes a resize to always be reported after the first `Tick()` call.
        uint64_t exit_request_time = 0;

        std::string text_input;

        ivec2 mouse_pos = ivec2(0);
        ivec2 mouse_movement = ivec2(0);

        bool keyboard_focus = false, mouse_focus = false;

        std::vector<InputTimes> input_times;

        std::vector<std::string> dropped_files, dropped_strings;


        Data() {}
        Data(const Data &) = delete;
        Data &operator=(const Data &) = delete;

        // The destructor will do cleanup only if this is true.
        bool is_complete = false;

        ~Data()
        {
            if (is_complete)
            {
                SDL_GL_DeleteContext(context);
                SDL_DestroyWindow(handle);
                SDL_Quit();
            }
        }
    };

    Window::~Window() {}

    Window::Window(std::string title, ivec2 size, FullscreenMode mode, const WindowSettings &settings)
    {
        constexpr const char *extra_error_details =
            PLATFORM_IF(pc)
            (
                "\n"
                "If you have several video cards, change your video driver settings\n"
                "to make it use the best available video card for this application.\n"
                "If it doesn't help, try updating your video card driver.\n"
                "If it doesn't help as well, your video card is probably too old to run this application.";
            )
            PLATFORM_IF(mobile)
            (
                "\n"
                "Your device doesn't support this application.";
            )

        // Stop if a window already exists.
        // This is probably not thread-safe.
        if (global_data.lock())
            Program::Error("Attempt to create multiple windows.");

        // Allocate state
        global_data = data = std::make_shared<Data>();
        FINALLY_ON_THROW( data = nullptr; )
        FINALLY_ON_SUCCESS( data->is_complete = true; )

        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
            Program::Error(STR("Unable to initialize SDL.\nMessage: ", (SDL_GetError())));
        FINALLY_ON_THROW( SDL_Quit(); )

        // Position
        ivec2 pos = settings.pos;
        if (pos == PosUndefined())
            pos = ivec2(SDL_WINDOWPOS_UNDEFINED_DISPLAY(settings.display));
        else if (pos == PosCentered())
            pos = ivec2(SDL_WINDOWPOS_CENTERED_DISPLAY(settings.display));

        // GL version
        if (settings.gl_major != 0 || settings.gl_minor != 0)
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, settings.gl_major);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, settings.gl_minor);
        }

        // GL profile
        switch (settings.gl_profile)
        {
          case Profile::core:
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            break;
          case Profile::compat:
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
            break;
          case Profile::es:
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            break;
          case Profile::any:
            // Do nothing.
            break;
        }

        // Hardware acceleration
        switch (settings.hw_accelerated)
        {
          case yes:
            SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
            break;
          case no:
            SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 0);
            break;
          case dont_care:
            // Do nothing.
            break;
        }

        // Antialiasing
        if (settings.msaa > 1)
        {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, settings.msaa);
        }

        // Bit depth
        if (settings.color_bits.r) SDL_GL_SetAttribute(SDL_GL_RED_SIZE    , settings.color_bits.r);
        if (settings.color_bits.g) SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE  , settings.color_bits.g);
        if (settings.color_bits.b) SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE   , settings.color_bits.b);
        if (settings.color_bits.a) SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE  , settings.color_bits.a);
        if (settings.depth_bits  ) SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE  , settings.depth_bits  );
        if (settings.stencil_bits) SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, settings.stencil_bits);

        // Context flags (debug-ness and forward compatibility)
        uint32_t context_flags = 0;
        if (settings.gl_debug)
            context_flags |= SDL_GL_CONTEXT_DEBUG_FLAG;
        if (settings.fwd_compat)
            context_flags |= SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, context_flags);

        // Window flags (resizability)
        uint32_t window_flags = SDL_WINDOW_OPENGL;
        if (!settings.fixed_size)
            window_flags |= SDL_WINDOW_RESIZABLE;

        // Create the window
        data->handle = SDL_CreateWindow(title.c_str(), pos.x, pos.y, size.x, size.y, window_flags);
        if (!data->handle)
            Program::Error("Unable to create a window with following properties:\n", settings.Summary(), extra_error_details);
        FINALLY_ON_THROW( SDL_DestroyWindow(data->handle); )

        // Get an appropriate display mode for fullscreen.
        SDL_DisplayMode display_mode{};
        bool have_display_mode = false;
        if (settings.fixed_size) // If window is not resizable, get a display mode closest to the window resolution
        {
            display_mode.w = size.x;
            display_mode.h = size.y;
            // From looking at the code, passing the same pointer twice should be ok.
            have_display_mode = bool(SDL_GetClosestDisplayMode(settings.display, &display_mode, &display_mode));
        }
        if (!have_display_mode) // If the window is resizable, or if a closest mode couldn't be obtained, try using the desktop mode.
        {
            have_display_mode = !SDL_GetDesktopDisplayMode(settings.display, &display_mode);
        }
        if (have_display_mode) // If we have an appropriate mode, set it. Otherwise do nothing and hope for the best.
        {
            // This function can fail, but there is nothing we can do anyway.
            SDL_SetWindowDisplayMode(data->handle, &display_mode);
        }

        // Create the context
        data->context = SDL_GL_CreateContext(data->handle);
        if (!data->context)
            Program::Error("Unable to create an OpenGL context with following properties:\n", settings.Summary(), extra_error_details);
        FINALLY_ON_THROW( SDL_GL_DeleteContext(data->context); )

        // Set the minimal size
        if (settings.min_size)
            SDL_SetWindowMinimumSize(data->handle, settings.min_size.x, settings.min_size.y);

        // Set vsync mode
        SetVSyncMode(settings.vsync);

        // Save resizability flag
        data->resizable = !settings.fixed_size;

        // Save title
        data->title = std::move(title);

        // Set fullscreen mode.
        // I had some problems with passing the mode as a window flag, so we set it here.
        if (mode != windowed)
            SetMode(mode); // This sets `data->mode`.

        // Get current window size
        SDL_GetWindowSize(data->handle, &data->size.x, &data->size.y);

        // Load OpenGL functions
        cglfl::load(SDL_GL_GetProcAddress);

        // Allocate input array
        data->input_times = std::vector<InputTimes>(Input::IndexCount);
    }

    SDL_Window *Window::GetHandleOrNull()
    {
        if (auto ptr = global_data.lock())
            return ptr->handle;
        else
            return nullptr;
    }

    bool Window::IsOpen()
    {
        return !global_data.expired();
    }

    Window Window::Get()
    {
        if (auto ptr = global_data.lock())
        {
            Window ret;
            ret.data = std::move(ptr);
            return ret;
        }
        else
        {
            Program::Error("Attempt to use the window before it was created.");
        }
    }

    SDL_Window *Window::Handle() const
    {
        return data->handle;
    }

    SDL_GLContext Window::Context() const
    {
        return data->context;
    }

    void Window::SetTitle(std::string new_title)
    {
        if (new_title == data->title)
            return;
        SDL_SetWindowTitle(data->handle, new_title.c_str());
        data->title = std::move(new_title);
    }

    const std::string &Window::Title() const
    {
        return data->title;
    }

    ivec2 Window::Size() const
    {
        return data->size;
    }

    bool Window::Resizable() const
    {
        return data->resizable;
    }

    void Window::SetVSyncMode(VSync new_vsync)
    {
        // Attempts to set a vsync mode. Returns true on success.
        auto TryMode = [&](VSync new_vsync) -> bool
        {
            bool failed = false;
            switch (new_vsync)
            {
              case VSync::disabled:
                failed = SDL_GL_SetSwapInterval(0);
                break;
              case VSync::enabled:
                failed = SDL_GL_SetSwapInterval(1);
                break;
              case VSync::adaptive:
                failed = SDL_GL_SetSwapInterval(-1);
                break;
              case VSync::unspecified:
                // Do nothing.
                break;
            }

            if (!failed)
            {
                data->vsync = new_vsync;
                return true;
            }
            else
            {
                return false;
            }
        };

        switch (new_vsync)
        {
          case VSync::disabled:
            if (TryMode(VSync::disabled)) return;
            if (TryMode(VSync::enabled )) return;
            break;
          case VSync::enabled:
            if (TryMode(VSync::enabled )) return;
            if (TryMode(VSync::disabled)) return;
            break;
          case VSync::adaptive:
            if (TryMode(VSync::adaptive)) return;
            if (TryMode(VSync::enabled )) return;
            if (TryMode(VSync::disabled)) return;
            break;
          case VSync::unspecified:
            // Do nothing.
            break;
        }

        TryMode(VSync::unspecified);
    }

    VSync Window::VSyncMode() const
    {
        return data->vsync;
    }

    void Window::SetMode(FullscreenMode new_mode)
    {
        if (new_mode == borderless_fullscreen && !data->resizable)
            new_mode = fullscreen; // Borderless fullscreen would force a window resize even if it's not normally resiable. So we use a normal fullscreen mode instead.

        int mode_value;
        switch (new_mode)
        {
          case windowed:
          default:
            mode_value = 0;
            break;
          case fullscreen:
            mode_value = SDL_WINDOW_FULLSCREEN;
            break;
          case borderless_fullscreen:
            mode_value = SDL_WINDOW_FULLSCREEN_DESKTOP;
            break;
        }

        if (SDL_SetWindowFullscreen(data->handle, mode_value))
            return; // We silently ignore a failure.

        data->mode = new_mode;
    }

    FullscreenMode Window::Mode() const
    {
        return data->mode;
    }

    void Window::ProcessEvents(const std::vector<std::function<bool(SDL_Event &)>> &hooks)
    {
        data->tick_counter++;

        data->text_input.clear();
        data->mouse_movement = ivec2(0);

        data->dropped_files.clear();
        data->dropped_strings.clear();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            bool drop_event = false;

            for (auto &hook : hooks)
            {
                if (!hook(event))
                {
                    drop_event = true;
                    break;
                }
            }

            if (drop_event)
                continue;

            switch (event.type)
            {
              case SDL_QUIT:
                data->exit_request_time = data->tick_counter;
                break;

              case SDL_KEYDOWN:
                {
                    int index = event.key.keysym.scancode;
                    if (index < Input::BeginKeys || index >= Input::EndKeys || index == 0)
                        break;
                    data->input_times[Input::BeginKeys + index].repeat = data->tick_counter;
                    if (event.key.repeat)
                        break;
                    data->input_times[Input::BeginKeys + index].press = data->tick_counter;
                }
                break;
              case SDL_KEYUP:
                {
                    int index = event.key.keysym.scancode;
                    if (index < Input::BeginKeys || index >= Input::EndKeys || index == 0)
                        break;
                    if (event.key.repeat) // We don't care about repeated releases.
                        break;
                    data->input_times[Input::BeginKeys + index].release = data->tick_counter;
                }
                break;

              case SDL_MOUSEBUTTONDOWN:
                {
                    int index = event.button.button;
                    if (index == 0)
                        break;
                    index--;
                    if (index >= Input::EndMouseButtons - Input::BeginMouseButtons)
                        break;
                    data->input_times[Input::BeginMouseButtons + index].press = data->tick_counter;
                    data->input_times[Input::BeginMouseButtons + index].repeat = data->tick_counter;
                }
                break;
              case SDL_MOUSEBUTTONUP:
                {
                    int index = event.button.button;
                    if (index == 0)
                        break;
                    index--;
                    if (index >= Input::EndMouseButtons - Input::BeginMouseButtons)
                        break;
                    data->input_times[Input::BeginMouseButtons + index].release = data->tick_counter;
                }
                break;

              case SDL_MOUSEWHEEL:
                if (event.wheel.x != 0 || event.wheel.y != 0)
                {
                    Input::Enum wheel_enum;
                    if (abs(event.wheel.y) > abs(event.wheel.x))
                        wheel_enum = event.wheel.y > 0 ? Input::mouse_wheel_up : Input::mouse_wheel_down; // Note `y > 0` is up.
                    else
                        wheel_enum = event.wheel.x > 0 ? Input::mouse_wheel_right : Input::mouse_wheel_left;

                    data->input_times[wheel_enum].press = data->tick_counter;
                    data->input_times[wheel_enum].release = data->tick_counter;
                    data->input_times[wheel_enum].repeat = data->tick_counter;
                }
                break;

              case SDL_MOUSEMOTION:
                data->mouse_pos = ivec2(event.motion.x, event.motion.y);
                data->mouse_movement += ivec2(event.motion.xrel, event.motion.yrel);
                break;

              case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                  case SDL_WINDOWEVENT_CLOSE:
                    data->exit_request_time = data->tick_counter;
                    break;
                  case SDL_WINDOWEVENT_SIZE_CHANGED:
                    if (data->tick_counter == 1)
                        break;
                    data->resize_time = data->tick_counter;
                    SDL_GetWindowSize(data->handle, &data->size.x, &data->size.y);
                    break;
                }
                break;

              case SDL_TEXTINPUT:
                data->text_input += event.text.text;
                break;

              case SDL_DROPFILE:
              case SDL_DROPTEXT:
                if (event.drop.file != 0)
                {
                    FINALLY( SDL_free(event.drop.file); )

                    if (event.type == SDL_DROPFILE)
                        data->dropped_files.emplace_back(event.drop.file);
                    else
                        data->dropped_strings.emplace_back(event.drop.file);
                }
            }
        }

        data->keyboard_focus = SDL_GetKeyboardFocus() == data->handle;
        data->mouse_focus = SDL_GetMouseFocus() == data->handle;
    }

    void Window::SwapBuffers()
    {
        data->frame_counter++;
        SDL_GL_SwapWindow(data->handle);
    }

    uint64_t Window::Ticks() const
    {
        return data->tick_counter;
    }

    uint64_t Window::Frames() const
    {
        return data->frame_counter;
    }

    bool Window::ExitRequested() const
    {
        return data->exit_request_time == data->tick_counter;
    }

    bool Window::Resized() const
    {
        return data->resize_time == data->tick_counter;
    }

    bool Window::HasKeyboardFocus() const
    {
        return data->keyboard_focus;
    }

    bool Window::HasMouseFocus() const
    {
        return data->mouse_focus;
    }

    const std::string &Window::TextInput() const
    {
        return data->text_input;
    }

    Window::InputTimes Window::GetInputTimes(Input::Enum index) const
    {
        if (index <= 0/*sic*/ || index >= Input::IndexCount)
            return {};

        return data->input_times[index];
    }

    ivec2 Window::MousePos() const
    {
        return data->mouse_pos;
    }

    ivec2 Window::MouseMovement() const
    {
        return data->mouse_movement;
    }

    void Window::HideCursor(bool hide)
    {
        SDL_ShowCursor(!hide);
    }

    void Window::RelativeMouseMode(bool relative)
    {
        SDL_SetRelativeMouseMode(SDL_bool(relative));
    }

    const std::vector<std::string> &Window::DroppedFiles()
    {
        return data->dropped_files;
    }

    const std::vector<std::string> &Window::DroppedStrings()
    {
        return data->dropped_strings;
    }
}
