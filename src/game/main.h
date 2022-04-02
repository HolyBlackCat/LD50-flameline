#pragma once

extern const ivec2 screen_size;
extern const std::string_view window_name;

extern Interface::Window window;

extern Audio::Context audio_context;
extern Audio::SourceManager audio_controller;

extern const Graphics::ShaderConfig shader_config;
extern Interface::ImGuiController gui_controller;

namespace Fonts
{
    namespace Files
    {
        extern Graphics::FontFile main;
    }

    extern Graphics::Font main;
}

extern Graphics::TextureAtlas texture_atlas;

extern Graphics::Texture texture_main;

extern GameUtils::AdaptiveViewport adaptive_viewport;
extern Render r;

extern Input::Mouse mouse;

extern Random::DefaultGenerator random_generator;
extern Random::DefaultInterfaces<Random::DefaultGenerator> ra;

STRUCT( StateBase EXTENDS GameUtils::State::Base POLYMORPHIC )
{
    virtual void Render() const = 0;
};
