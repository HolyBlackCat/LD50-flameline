#pragma once

#include <utility>

#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <imgui_stdlib.h>

#include "interface/window.h"
#include "program/errors.h"
#include "utils/finally.h"

namespace Interface
{
    class ImGuiController
    {
      public:
        enum class Backend
        {
            opengl_fixed_function,
            opengl_modern,
        };

      private:
        struct Data
        {
            ImGuiContext *context = 0;
            bool frame_started = 0;
            bool frame_rendered = 0;
            Backend backend = Backend::opengl_modern;
        };
        Data data;

      public:
        ImGuiController() {}

        ImGuiController(Backend backend, std::string shader_header, bool save_gui_state = 1) // `shader_header` is ignored if the selected backend doesn't use shaders.
        {
            data.backend = backend;

            // Initialize ImGui.
            IMGUI_CHECKVERSION();

            data.context = ImGui::CreateContext();
            if (!data.context)
                Program::Error("Unable to create an ImGui context.");
            FINALLY_ON_THROW( ImGui::DestroyContext(data.context); )

            if (!ImGui_ImplSDL2_InitForOpenGL(Window::Get().Handle(), Window::Get().Context()))
                Program::Error("Unable to initialize ImGui SDL2 backend.");
            FINALLY_ON_THROW( ImGui_ImplSDL2_Shutdown(); )

            bool backend_ok = 0;
            switch (backend)
            {
              case Backend::opengl_fixed_function:
                backend_ok = ImGui_ImplOpenGL2_Init();
                break;
              case Backend::opengl_modern:
                backend_ok = ImGui_ImplOpenGL3_Init(shader_header.c_str());
                break;
            }
            if (!backend_ok)
                Program::Error("Unable to initialize ImGui OpenGL backend.");
            FINALLY_ON_THROW(
                switch (backend)
                {
                  case Backend::opengl_fixed_function:
                    ImGui_ImplOpenGL2_Shutdown();
                    break;
                  case Backend::opengl_modern:
                    ImGui_ImplOpenGL3_Shutdown();
                    break;
                }
            )

            // Activate context.
            Activate();

            // Disable saving GUI state to file if necessary.
            if (!save_gui_state)
                ImGui::GetIO().IniFilename = 0;
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

                switch (data.backend)
                {
                  case Backend::opengl_fixed_function:
                    ImGui_ImplOpenGL2_Shutdown();
                    break;
                  case Backend::opengl_modern:
                    ImGui_ImplOpenGL3_Shutdown();
                    break;
                }

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
            if (!data.context)
                Program::Error("Attempt to use a null ImGui context.");
            ImGui::SetCurrentContext(data.context);
        }

        bool IsActive()
        {
            return data.context && data.context == ImGui::GetCurrentContext();
        }

        auto EventHook() // Use this with `Window::ProcessEvents()`.
        {
            return [this](SDL_Event &event)
            {
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
                    return 0;

                // Discard mouse events if the mouse is captured.
                // Note that we don't discard `SDL_MOUSEBUTTONUP` to prevent mouse buttons from getting stuck.
                if (ImGui::GetIO().WantCaptureMouse && (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEWHEEL))
                    return 0;

                return 1;
            };
        }

        void PreTick()
        {
            if (data.frame_started)
                ImGui::EndFrame();

            switch (data.backend)
            {
              case Backend::opengl_fixed_function:
                ImGui_ImplOpenGL2_NewFrame();
                break;
              case Backend::opengl_modern:
                ImGui_ImplOpenGL3_NewFrame();
                break;
            }

            ImGui_ImplSDL2_NewFrame(Window::Get().Handle());
            ImGui::NewFrame();

            data.frame_started = 1;
        }

        void PreRender()
        {
            if (data.frame_started)
            {
                data.frame_started = 0;
                ImGui::Render();
                data.frame_rendered = 1;
            }
        }

        void PostRender()
        {
            if (data.frame_rendered)
            {
                // We never set `frame_rendered` back to 0. It's sole purpose is to avoid segfault on the first frame.

                switch (data.backend)
                {
                  case Backend::opengl_fixed_function:
                    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
                    break;
                  case Backend::opengl_modern:
                    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                    break;
                }
            }
        }
    };
}
