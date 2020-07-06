#pragma once

#include "game/master.hpp"

inline constexpr ivec2 screen_size(480, 270);
extern Interface::Window window;
extern const Graphics::ShaderConfig shader_config;
extern Interface::ImGuiController gui_controller;

namespace Fonts
{
    namespace Files
    {
        Graphics::FontFile &Main();
    }

    Graphics::Font &Main();
}

Graphics::TextureAtlas &TextureAtlas();
extern Graphics::Texture texture_main;
extern AdaptiveViewport adaptive_viewport;
extern Render r;
extern Input::Mouse mouse;
