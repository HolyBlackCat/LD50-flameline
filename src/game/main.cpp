#include "main.h"

constexpr bool is_debug =
#ifdef NDEBUG
    false;
#else
    true;
#endif

bool now_windowed = is_debug;
constexpr auto fullscreen_flavor = Interface::borderless_fullscreen;

const std::string_view window_name = "Flameline";

Interface::Window window(std::string(window_name), screen_size * 2, now_windowed ? Interface::windowed : fullscreen_flavor, adjust_(Interface::WindowSettings{}, min_size = screen_size));
static Graphics::DummyVertexArray dummy_vao = nullptr;

Audio::Context audio_context = nullptr;
Audio::SourceManager audio_controller;

const Graphics::ShaderConfig shader_config = Graphics::ShaderConfig::Core();

Graphics::FontFile Fonts::Files::main(Program::ExeDir() + "assets/Monocat_7x14.ttf", 14);
Graphics::Font Fonts::main;

Graphics::TextureAtlas texture_atlas = []{
    std::string atlas_loc = is_debug ? "assets/assets" : Program::ExeDir() + "assets/";
    Graphics::TextureAtlas ret(ivec2(2048), is_debug ? "assets/_images" : "", atlas_loc + "atlas.png", atlas_loc + "atlas.refl", {{"/font_storage", ivec2(256)}});
    auto font_region = ret.Get("/font_storage");

    Unicode::CharSet glyph_ranges;
    glyph_ranges.Add(Unicode::Ranges::Basic_Latin);

    Graphics::MakeFontAtlas(ret.GetImage(), font_region.pos, font_region.size, {
        {Fonts::main, Fonts::Files::main, glyph_ranges, Graphics::FontFile::monochrome_with_hinting},
    });
    return ret;
}();
Graphics::Texture texture_main = Graphics::Texture(nullptr).Wrap(Graphics::clamp).Interpolation(Graphics::nearest).SetData(texture_atlas.GetImage());

GameUtils::AdaptiveViewport adaptive_viewport(shader_config, screen_size);
Render r = adjust_(Render(0x2000, shader_config), SetTexture(texture_main), SetMatrix(adaptive_viewport.GetDetails().MatrixCentered()));

Input::Mouse mouse;

Random::DefaultGenerator random_generator = Random::MakeGeneratorFromRandomDevice();
Random::DefaultInterfaces<Random::DefaultGenerator> ra(random_generator);

namespace Theme
{
    Audio::Buffer buf(Audio::Sound(Audio::ogg, Audio::stereo, Program::ExeDir() + "assets/gates_of_heck.ogg"));
    Audio::Source src = adjust_(Audio::Source(buf), loop(), play());
}

struct Application : Program::DefaultBasicState
{
    GameUtils::State::Manager<StateBase> state_manager;
    GameUtils::FpsCounter fps_counter;

    void Resize()
    {
        adaptive_viewport.Update();
        mouse.SetMatrix(adaptive_viewport.GetDetails().MouseMatrixCentered());
    }

    Metronome metronome = Metronome(60);

    Metronome *GetTickMetronome() override
    {
        return &metronome;
    }

    int GetFpsCap() override
    {
        return 60 * NeedFpsCap();
    }

    void EndFrame() override
    {
        fps_counter.Update();
        if (is_debug)
            window.SetTitle(STR((window_name), " TPS:", (fps_counter.Tps()), " FPS:", (fps_counter.Fps()), " AUDIO:", (audio_controller.ActiveSources())));
    }

    void Tick() override
    {
        window.ProcessEvents();

        if (window.ExitRequested())
            Program::Exit();
        if (window.Resized())
        {
            Resize();
            Graphics::Viewport(window.Size());
        }

        state_manager.Tick();
        audio_controller.Tick();

        Audio::CheckErrors();

        if (!state_manager)
            Program::Exit();

        // Toggle fullscreen.
        if ((Input::Button(Input::l_alt).down() || Input::Button(Input::r_alt).down()) && Input::Button(Input::enter).pressed())
        {
            now_windowed = !now_windowed;
            window.SetMode(now_windowed ? Interface::windowed : fullscreen_flavor);
        }

        // Toggle music.
        if (Input::Button(Input::m).pressed())
        {
            if (Theme::src.IsPlaying())
                Theme::src.pause();
            else
                Theme::src.play();
        }
    }

    void Render() override
    {
        adaptive_viewport.BeginFrame();
        state_manager.Call(&StateBase::Render);
        adaptive_viewport.FinishFrame();
        Graphics::CheckErrors();

        window.SwapBuffers();
    }


    void Init()
    {
        mouse.HideCursor();

        Audio::LoadMentionedFiles(Audio::LoadFromPrefixWithExt(Program::ExeDir() + "assets/"), Audio::mono, Audio::wav);

        if (is_debug)
            SDL_MaximizeWindow(window.Handle());

        Graphics::Blending::Enable();
        Graphics::Blending::FuncNormalPre();

        state_manager.SetState("World{}");
    }
};

IMP_MAIN(,)
{
    Application app;
    app.Init();
    app.Resize();
    app.RunMainLoop();
    return 0;
}
