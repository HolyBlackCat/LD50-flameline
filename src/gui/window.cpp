#include "window.h"

#include <string>
#include <utility>

#include <GLFL/glfl.h>

#include "errors.h"
#include "strings.h"


// Export some variables to advise video drivers to use the best available video card for the application.
OnPC
(
    extern "C"
    {
        __declspec(dllexport) uint32_t NvOptimusEnablement                  = 1; // For NVidia. Docs suggest that this should be of type DWORD, but I don't want windows headers here.
        __declspec(dllexport) int      AmdPowerXpressRequestHighPerformance = 1; // For AMD.
    }
)

namespace GUI
{
    std::string Window::Settings::GetSummary() const // Returns a short readable summary of window settings. Settings that can't cause a window creation error aren't included.
    {
        std::string ret;

        if (gl_major || gl_minor)
            ret = Str("OpenGL ", gl_major, ".", gl_minor);
        else
            ret = "Any OpenGL version";

        switch (gl_profile)
        {
            case Profile::core: ret += ", core profile"; break;
            case Profile::compatibility: ret += ", compatibility profile"; break;
            case Profile::es: ret += ", ES profile"; break;
            case Profile::any_profile: /* nothing */ break;
        }

        if (gl_debug)
            ret += ", debug mode";

        if (forward_compatibility)
            ret += ", forward compatibility enabled";

        switch (hardware_acceleration)
        {
            case yes: ret += ", hardware acceleration enabled"; break;
            case no: ret += ", hardware acceleration disabled"; break;
            case dont_care: /* nothing */ break;
        }

        if (msaa > 1)
            ret += Str(", ", msaa, "x MSAA");

        if (color_bits != 0 || depth_bits != 0 || stencil_bits != 0)
        {
            ret += ", bits per pixel:";
            if (color_bits.r != 0) ret += Str(" r=", color_bits.r);
            if (color_bits.g != 0) ret += Str(" g=", color_bits.g);
            if (color_bits.b != 0) ret += Str(" b=", color_bits.b);
            if (color_bits.a != 0) ret += Str(" a=", color_bits.a);
            if (depth_bits   != 0) ret += Str(" depth=", depth_bits);
            if (stencil_bits != 0) ret += Str(" stencil=", stencil_bits);
        }

        return ret;
    }

    struct Window::Data
    {
        SDL_Window *handle = 0;
        SDL_GLContext context = 0;

        ivec2 size = ivec2(0);
        VSync vsync = VSync::unspecified;
        bool resizable = 0;
        FullscreenMode mode = FullscreenMode::windowed;

        Data(const Data &) = delete;
        Data &operator=(const Data &) = delete;

        Data(std::string name, ivec2 new_size, FullscreenMode new_mode, const Settings &settings)
        {
            try
            {
                // Stop if a window already exists
                if (instance)
                    Program::Error("Attempt to create multiple windows.");

                // Initialize SDL
                static bool sdl_initialized = 0;
                if (!sdl_initialized)
                {
                    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
                        Program::Error(Str("Unable to initialize SDL.\nMessage: ", SDL_GetError()));
                    sdl_initialized = 1;
                }

                // Position
                ivec2 pos;
                switch (settings.position)
                {
                  case Position::custom:
                    pos = settings.coords;
                    break;
                  case Position::centered:
                    pos = ivec2(SDL_WINDOWPOS_CENTERED_DISPLAY(settings.display));
                    break;
                  case Position::undefined:
                    pos = ivec2(SDL_WINDOWPOS_UNDEFINED_DISPLAY(settings.display));
                    break;
                }

                // GL version
                if (settings.gl_major || settings.gl_minor)
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
                  case Profile::compatibility:
                    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
                    break;
                  case Profile::es:
                    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
                    break;
                  default:
                    break;
                }

                // Hardware acceleration
                switch (settings.hardware_acceleration)
                {
                  case yes:
                    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
                    break;
                  case no:
                    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 0);
                    break;
                  default:
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
                if (settings.forward_compatibility)
                    context_flags |= SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, context_flags);

                // Window flags (resizability)
                uint32_t window_flags = SDL_WINDOW_OPENGL;
                if (settings.resizable)
                    window_flags |= SDL_WINDOW_RESIZABLE;

                // Create the window
                handle = SDL_CreateWindow(name.c_str(), pos.x, pos.y, new_size.x, new_size.y, window_flags);
                if (!handle)
                    Program::Error("Unable to create a window with following properties:\n",
                                   settings.GetSummary(), "\n"
                                   OnPC("If you have several video cards, change your video driver settings\n"
                                        "to make it use the best available video card for this application."
                                        "If it didn't help, try updating your video card driver.\n"
                                        "If it didn't help as well, your video card is probably too old to run this application.")
                                   );

                // Get an appropriate display mode for fullscreen.
                SDL_DisplayMode display_mode{};
                bool have_display_mode = 0;
                if (!settings.resizable) // If window is not resizable, get a display mode closest to it's resolution
                {
                    display_mode.w = new_size.x;
                    display_mode.h = new_size.y;
                    have_display_mode = bool(SDL_GetClosestDisplayMode(settings.display, &display_mode, &display_mode));
                }
                if (settings.resizable) // If the window is resizable, or if a closest mode couldn't be obtained, try using the desktop mode.
                {
                    have_display_mode = !SDL_GetDesktopDisplayMode(settings.display, &display_mode);
                }
                if (have_display_mode)
                    SDL_SetWindowDisplayMode(handle, &display_mode); // If we have an appropriate mode, set it. This function can fail, but there is nothing we can do anyway.

                // Create the context
                context = SDL_GL_CreateContext(handle);
                if (!context)
                    if (!handle)
                    Program::Error("Unable to create an OpenGL context with following properties:\n",
                                   settings.GetSummary(), "\n"
                                   OnPC("If you have several video cards, change your video driver settings\n"
                                        "to make it use the best available video card for this application."
                                        "If it didn't help, try updating your video card driver.\n"
                                        "If it didn't help as well, your video card is probably too old to run this application.")
                                   );

                // Set the minimal size
                if (settings.min_size)
                    SDL_SetWindowMinimumSize(handle, settings.min_size.x, settings.min_size.y);

                // Set vsync mode
                vsync = settings.vsync_mode;
                switch (vsync)
                {
                  case VSync::disabled:
                    if (SDL_GL_SetSwapInterval(0))
                    {
                        vsync = VSync::enabled;
                        if (SDL_GL_SetSwapInterval(1))
                            vsync = VSync::unspecified;
                    }
                    break;
                  case VSync::enabled:
                    if (SDL_GL_SetSwapInterval(1))
                    {
                        vsync = VSync::disabled;
                        if (SDL_GL_SetSwapInterval(0))
                            vsync = VSync::unspecified;
                    }
                    break;
                  case VSync::adaptive:
                    if (SDL_GL_SetSwapInterval(-1))
                    {
                        vsync = VSync::enabled;
                        if (SDL_GL_SetSwapInterval(1))
                        {
                            vsync = VSync::disabled;
                            if (SDL_GL_SetSwapInterval(0))
                                vsync = VSync::unspecified;
                        }
                    }
                    break;
                  case VSync::unspecified:
                    break;
                }

                // Save resizability flag
                resizable = settings.resizable;

                // Set fullscreen mode
                if (new_mode != windowed)
                    SetMode(new_mode); // This sets `mode`.

                // Get current window size
                SDL_GetWindowSize(handle, &size.x, &size.y);

                // Load OpenGL functions
                glfl::set_function_loader(SDL_GL_GetProcAddress);
                if (settings.gl_profile != Profile::es)
                    glfl::load_gl(settings.gl_major, settings.gl_minor);
                else
                    glfl::load_gles(settings.gl_major, settings.gl_minor);
            }
            catch (std::exception &e)
            {
                if (context)
                    SDL_GL_DeleteContext(context);
                if (handle)
                    SDL_DestroyWindow(handle);
            }
        }

        ~Data()
        {
            SDL_GL_DeleteContext(context);
            SDL_DestroyWindow(handle);
        }
    };

    Window::Window() {}

    Window::Window(Window &&other) noexcept : data(std::move(other.data))
    {
        if (instance == &other)
            instance = this;
    }

    Window &Window::operator=(Window &&other) noexcept
    {
        data = std::move(other.data);
        if (instance == &other)
            instance = this;
        return *this;
    }

    Window::~Window()
    {
        if (instance == this)
            instance = 0;
    }

    Window::Window(std::string name, ivec2 size, FullscreenMode mode, const Settings &settings)
    {
        data = std::make_unique<Data>(name, size, mode, settings);
        instance = this;
    }

    ivec2 Window::Size()
    {
        if (!instance)
            return ivec2(0);
        return instance->data->size;
    }

    SDL_Window *Window::Handle()
    {
        if (!instance)
            return 0;
        return instance->data->handle;
    }

    SDL_GLContext Window::Context()
    {
        if (!instance)
            return 0;
        return instance->data->context;
    }

    Window::VSync Window::VSyncMode()
    {
        if (!instance)
            return VSync::unspecified;
        return instance->data->vsync;
    }

    bool Window::Resizable()
    {
        if (!instance)
            return 0;
        return instance->data->resizable;
    }

    void Window::SetMode(FullscreenMode new_mode)
    {
        if (!instance)
            return;

        if (new_mode == borderless_fullscreen && instance->data->resizable == 0)
            new_mode = fullscreen; // Borderless fullscreen would force a window resize even if it's not normally resiable. So we use a normal fullscreen mode instead.

        int mode_value;
        switch (new_mode)
        {
          case windowed:
            mode_value = 0;
            break;
          case fullscreen:
            mode_value = SDL_WINDOW_FULLSCREEN;
            break;
          case borderless_fullscreen:
            mode_value = SDL_WINDOW_FULLSCREEN_DESKTOP;
            break;
        }

        if (SDL_SetWindowFullscreen(instance->data->handle, mode_value))
            return; // We silently ignore a failure.

        instance->data->mode = new_mode;
    }

    Window::FullscreenMode Window::Mode()
    {
        if (!instance)
            return windowed;

        return instance->data->mode;
    }

    void Window::Swap()
    {
        if (!instance)
            return;

        SDL_GL_SwapWindow(instance->data->handle);
    }
}
