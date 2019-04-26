#pragma once

#include <utility>

#include <imgui.h>
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
        struct Data
        {
            ImGuiContext *context = 0;
            bool frame_started = 0;
            bool frame_rendered = 0;
        };
        Data data;

      public:
        ImGuiController() {}

        // Don't forget to set theme with `ImGui::StyleColors*()` after creating the controller.
        ImGuiController(std::string shader_header, bool save_gui_state = 1)
        {
            // Initialize ImGui.
            IMGUI_CHECKVERSION();

            data.context = ImGui::CreateContext();
            if (!data.context)
                Program::Error("Unable to create an ImGui context.");
            FINALLY_ON_THROW( ImGui::DestroyContext(data.context); )

            if (!ImGui_ImplSDL2_InitForOpenGL(Window::Get().Handle(), Window::Get().Context()))
                Program::Error("Unable to initialize ImGui SDL2 attachment.");
            FINALLY_ON_THROW( ImGui_ImplSDL2_Shutdown(); )

            if (!ImGui_ImplOpenGL3_Init(shader_header.c_str()))
                Program::Error("Unable to initialize ImGui OpenGL attachment.");
            FINALLY_ON_THROW( ImGui_ImplOpenGL3_Shutdown(); )

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
                if (IsActive())
                    ImGui::SetCurrentContext(0);

                ImGui_ImplOpenGL3_Shutdown();
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

            ImGui_ImplOpenGL3_NewFrame();
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
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }
        }
    };
}
