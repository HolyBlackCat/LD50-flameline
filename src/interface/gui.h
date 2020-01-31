#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <imgui_stdlib.h>

#include "interface/window.h"
#include "macros/finally.h"
#include "meta/misc.h"
#include "program/errors.h"
#include "stream/readonly_data.h"
#include "utils/poly_storage.h"

namespace Interface
{
    class ImGuiController
    {
      public:
        struct Config
        {
            std::string shader_header; // Ignored if backend doesn't use shaders.
            std::string store_state_in_file = "imgui.ini"; // Set to empty string to not store state.
        };

        class GraphicsBackend : Meta::with_virtual_destructor<GraphicsBackend>
        {
          public:
            virtual bool Init(const Config &) = 0; // Returns `false` on failure.
            virtual void Shutdown() = 0;
            virtual void NewFrame() = 0;
            virtual void RenderFrame(ImDrawData *data) = 0;
            virtual bool Reload() = 0; // Returns `false` on failure.
        };

        class GraphicsBackend_FixedFunction : public GraphicsBackend
        {
          public:
            bool Init(const Config &) override
            {
                return ImGui_ImplOpenGL2_Init();
            }
            void Shutdown() override
            {
                ImGui_ImplOpenGL2_Shutdown();
            }
            void NewFrame() override
            {
                ImGui_ImplOpenGL2_NewFrame();
            }
            void RenderFrame(ImDrawData *data) override
            {
                ImGui_ImplOpenGL2_RenderDrawData(data);
            }
            bool Reload() override
            {
                ImGui_ImplOpenGL2_DestroyDeviceObjects();
                return ImGui_ImplOpenGL2_CreateDeviceObjects();
            }
        };

        class GraphicsBackend_Modern : public GraphicsBackend
        {
          public:
            bool Init(const Config &config) override
            {
                return ImGui_ImplOpenGL3_Init(config.shader_header.c_str());
            }
            void Shutdown() override
            {
                ImGui_ImplOpenGL3_Shutdown();
            }
            void NewFrame() override
            {
                ImGui_ImplOpenGL3_NewFrame();
            }
            void RenderFrame(ImDrawData *data) override
            {
                ImGui_ImplOpenGL3_RenderDrawData(data);
            }
            bool Reload() override
            {
                ImGui_ImplOpenGL3_DestroyDeviceObjects();
                return ImGui_ImplOpenGL3_CreateDeviceObjects();
            }
        };


      private:
        struct Data
        {
            ImGuiContext *context = 0;

            bool frame_started = 0;
            bool frame_rendered = 0;

            std::vector<std::function<void()>> execute_before_next_frame;

            Poly::Storage<GraphicsBackend> graphics_backend;
            // We need `unique_ptr` because ImGui stores the file name in the context as `const char *`, so the string has to remain valid even if the controller is moved.
            std::unique_ptr<std::string> state_file_name;

            std::vector<Stream::ReadOnlyData> font_storage;
        };
        Data data;

      public:
        ImGuiController() {}

        ImGuiController(Poly::Storage<GraphicsBackend> graphics_backend, const Config &config)
        {
            data.graphics_backend = std::move(graphics_backend);

            // Initialize ImGui.
            IMGUI_CHECKVERSION();

            data.context = ImGui::CreateContext();
            if (!data.context)
                Program::Error("Unable to create an ImGui context.");
            FINALLY_ON_THROW( ImGui::DestroyContext(data.context); )

            if (!ImGui_ImplSDL2_InitForOpenGL(Window::Get().Handle(), Window::Get().Context()))
                Program::Error("Unable to initialize ImGui SDL2 backend.");
            FINALLY_ON_THROW( ImGui_ImplSDL2_Shutdown(); )

            if (!data.graphics_backend->Init(config))
                Program::Error("Unable to initialize ImGui OpenGL backend.");
            FINALLY_ON_THROW( data.graphics_backend->Shutdown(); )

            // Activate context.
            Activate();

            // Set file name.
            if (config.store_state_in_file.empty())
            {
                ImGui::GetIO().IniFilename = 0;
            }
            else
            {
                data.state_file_name = std::make_unique<std::string>(config.store_state_in_file);
                ImGui::GetIO().IniFilename = data.state_file_name->c_str();
            }
        }

        ImGuiController(ImGuiController &&other) noexcept : data(std::exchange(other.data, {})) {}
        ImGuiController &operator=(ImGuiController other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        ~ImGuiController()
        {
            if (data.context)
            {
                // We don't need to deactivate the context here, ImGui does it automatically if necessary.

                data.graphics_backend->Shutdown();
                ImGui_ImplSDL2_Shutdown();
                ImGui::DestroyContext(data.context);
            }
        }

        explicit operator bool() const
        {
            return bool(data.context);
        }

        void Activate()
        {
            if (!*this)
                Program::Error("Attempt to use a null ImGui controller.");
            ImGui::SetCurrentContext(data.context);
        }

        bool IsActive()
        {
            return data.context && data.context == ImGui::GetCurrentContext();
        }

        enum HookMode {block_events, pass_events};

        auto EventHook(HookMode mode = block_events) // Use this with `Window::ProcessEvents()`.
        {
            return [this, mode](SDL_Event &event) -> bool
            {
                bool false_if_blocking = mode != block_events;

                // Remember currently acitve context and activate this one instead.
                ImGuiContext *old_context = ImGui::GetCurrentContext();
                FINALLY( ImGui::SetCurrentContext(old_context); )
                Activate();

                // Handle event.
                bool event_used = ImGui_ImplSDL2_ProcessEvent(&event);
                if (!event_used)
                    return 1;

                // Discard keyboard events if the keyboard is captured.
                // Note that we don't discard `SDL_KEYUP` to prevent keys from getting stuck.
                if (ImGui::GetIO().WantCaptureKeyboard && (event.type == SDL_KEYDOWN || event.type == SDL_TEXTINPUT || event.type == SDL_TEXTEDITING))
                    return false_if_blocking;

                // Discard mouse events if the mouse is captured.
                // Note that we don't discard `SDL_MOUSEBUTTONUP` to prevent mouse buttons from getting stuck.
                if (ImGui::GetIO().WantCaptureMouse && (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEWHEEL))
                    return false_if_blocking;

                return 1;
            };
        }

        void PreTick()
        {
            ImGuiContext *old_context = ImGui::GetCurrentContext();
            FINALLY( ImGui::SetCurrentContext(old_context); )
            Activate();

            if (data.frame_started)
            {
                ImGui::EndFrame();
                data.frame_started = 0;
            }

            data.graphics_backend->NewFrame();
            ImGui_ImplSDL2_NewFrame(Window::Get().Handle());
            ImGui::NewFrame();

            data.frame_started = 1;
        }

        void PreRender()
        {
            ImGuiContext *old_context = ImGui::GetCurrentContext();
            FINALLY( ImGui::SetCurrentContext(old_context); )
            Activate();

            if (data.frame_started)
            {
                data.frame_started = 0;
                ImGui::Render();
                data.frame_rendered = 1;
            }
        }

        void PostRender()
        {
            ImGuiContext *old_context = ImGui::GetCurrentContext();
            FINALLY( ImGui::SetCurrentContext(old_context); )
            Activate();

            if (data.frame_rendered)
            {
                // Here we don't reset `frame_rendered` back to 0. Its sole purpose is to avoid segfault on the first frame.
                data.graphics_backend->RenderFrame(ImGui::GetDrawData());
            }
        }

        // Reload graphics backend.
        // Good for updating font settings.
        // Call this after rendering a frame, but before ticking.
        void ReloadGraphics()
        {
            if (!*this)
                Program::Error("Attempt to use a null ImGui controller.");

            if (data.frame_started)
                Program::Error("Unable to reload ImGui graphics backend now, frame rendering is in process.");

            if (!data.graphics_backend->Reload())
                Program::Error("Unable to reload ImGui graphics backend.");
        }

        // Load the default font.
        ImFont *LoadDefaultFont(ImFontConfig config = {})
        {
            ImGuiContext *old_context = ImGui::GetCurrentContext();
            FINALLY( ImGui::SetCurrentContext(old_context); )
            Activate();

            ImFont *ret = ImGui::GetIO().Fonts->AddFontDefault(&config);
            if (!ret)
                Program::Error("ImGui is unable to load the default font.");

            return ret;
        }

        // Load a font.
        ImFont *LoadFont(Stream::ReadOnlyData file, float size, ImFontConfig config = {}, const ImWchar *glyph_ranges = 0)
        {
            ImGuiContext *old_context = ImGui::GetCurrentContext();
            FINALLY( ImGui::SetCurrentContext(old_context); )
            Activate();

            config.FontDataOwnedByAtlas = 0;
            ImFont *ret = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t *>(file.data()), file.size(), size, &config, glyph_ranges);
            if (!ret)
                Program::Error("ImGui is unable to load font: `", file.name(), "`.");

            data.font_storage.push_back(std::move(file));
            return ret;
        }

        // Remove all fonts. You probably should call this between frames.
        void RemoveAllFonts()
        {
            ImGuiContext *old_context = ImGui::GetCurrentContext();
            FINALLY( ImGui::SetCurrentContext(old_context); )
            Activate();

            ImGui::GetIO().Fonts->Clear();
            data.font_storage = {};
        }

        // Replaces vanilla font renderer with freetype. Call this once, right after loading fonts. Use flags from `ImGuiFreeType::RasterizerFlags`.
        void RenderFontsWithFreetype(unsigned int global_flags = 0)
        {
            ImGuiContext *old_context = ImGui::GetCurrentContext();
            FINALLY( ImGui::SetCurrentContext(old_context); )
            Activate();

            ImGuiFreeType::BuildFontAtlas(ImGui::GetIO().Fonts, global_flags);
        }

        // Returns UTF8 code for a zero-width space.
        static const char *ZeroWidthSpace()
        {
            return "\xe2\x80\x8b";
        }

        // Adds a zero-width space after each '#' in the string.
        // ImGui treats "##" and "###" in widget titles in a special manner, use this function to prevent that.
        // Don't forget to add zero-width space to the character ranges when loading your fonts.
        static std::string EscapeStringForWidgetName(std::string source)
        {
            std::string ret;
            ret.reserve(source.size() + std::count(source.begin(), source.end(), '#'));
            for (char ch : source)
            {
                ret += ch;
                if (ch == '#')
                    ret += ZeroWidthSpace();
            }
            return ret;
        }
    };

    // A helper class to configure your fonts interactively.
    class ImGuiFreetypeFontConfigurator
    {
      public:
        using load_fonts_func_t = std::function<void(Interface::ImGuiController &controller, int font_size, int freetype_flags)>;

      private:
        Interface::ImGuiController *controller = nullptr;
        int font_size = 16;
        int freetype_flags = 0;
        load_fonts_func_t load_fonts_func;

        bool should_reload = false;

      public:
        ImGuiFreetypeFontConfigurator() {}

        // Calls the callback immediately. Also saves it, and calls it whenever font settings change.
        // Example callback:
        //     controller.LoadFont("assets/SourceSansPro-Regular.ttf", font_size, adjust(ImFontConfig{}, RasterizerFlags = freetype_flags));
        //     controller.LoadDefaultFont();
        //     controller.RenderFontsWithFreetype();
        ImGuiFreetypeFontConfigurator(Interface::ImGuiController &controller_ref, int font_size, ImGuiFreeType::RasterizerFlags default_flags, load_fonts_func_t new_func)
            : controller(&controller_ref), font_size(font_size), freetype_flags(default_flags), load_fonts_func(std::move(new_func))
        {
            load_fonts_func(*controller, font_size, freetype_flags);
        }

        explicit operator bool() const
        {
            return bool(controller);
        }

        // Draws the gui.
        void ShowWindow()
        {
            DebugAssert("Attempt to use a null instance of ImGuiFreetypeFontConfigurator.", *this);
            if (!*this)
                return;

            constexpr const char *flag_names[] = {"No hinting", "No auto-hint", "Force auto-hint", "Light hinting", "Mono hinting", "Bold", "Oblique", "Monochrome"};

            if (ImGui::Begin("Font config"))
            {
                if (ImGui::InputInt("Font size", &font_size, 1, 4))
                    should_reload = true;

                int this_flag = 1;
                for (const auto &flag_name : flag_names)
                {
                    bool checkbox = freetype_flags & this_flag;
                    if (ImGui::Checkbox(flag_name, &checkbox))
                        should_reload = true;

                    freetype_flags = (freetype_flags & ~this_flag) | (this_flag * checkbox);

                    this_flag <<= 1;
                }
            }

            ImGui::End();
        }

        // Updates font settings.
        // Call this before the tick loop, or after rendering.
        void UpdateFontsIfNecessary()
        {
            if (!should_reload)
                return;

            controller->RemoveAllFonts();
            load_fonts_func(*controller, font_size, freetype_flags);
            controller->ReloadGraphics();

            should_reload = false;
        }
    };
}
