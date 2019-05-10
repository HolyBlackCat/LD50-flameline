const ivec2 screen_size(480, 270);
Interface::Window window("Delta", screen_size * 2, Interface::windowed, Interface::WindowSettings{} with_(min_size = screen_size));
Graphics::DummyVertexArray dummy_vao;

const Graphics::ShaderConfig shader_config = Graphics::ShaderConfig::Core();
Interface::ImGuiController gui_controller(Poly::derived<Interface::ImGuiController::GraphicsBackend_Modern>, Interface::ImGuiController::Config{} with_(shader_header = shader_config.common_header));

TextureAtlas texture_atlas(ivec2(2048), "assets/_images", "assets/atlas.png", "assets/atlas.refl");
Graphics::Texture texture_main = Graphics::Texture().Wrap(Graphics::clamp).Interpolation(Graphics::nearest).SetData(texture_atlas.GetImage());

AdaptiveViewport adaptive_viewport(shader_config, screen_size);
Render r = Render(0x2000, shader_config) with_(SetTexture(texture_main), SetMatrix(adaptive_viewport.GetDetails().MatrixCentered()));

Input::Mouse mouse;

namespace States
{
    struct Base : Meta::with_virtual_destructor<Base>
    {
        virtual void Tick() = 0;
        virtual void Render() const = 0;
    };

    Poly::Storage<Base> current_state;

    struct Game : Base
    {
        void Tick() override
        {
            ImGui::ShowDemoWindow();
        }

        void Render() const override
        {
            Graphics::SetClearColor(fvec3(0));
            Graphics::Clear();

            r.BindShader();

            r.iquad(mouse.pos(), ivec2(32)).center().rotate(window.Ticks() / 100.).color(mouse.left.down() ? fvec3(1,0.5,0) : fvec3(0,0.5,1));

            r.Finish();
        }
    };
}

int main()
{
    { // Initialize
        ImGui::StyleColorsDark();

        // Load various small fonts
        gui_controller.LoadFont("assets/Monokat_6x12.ttf", 12.0f);
        gui_controller.LoadDefaultFont();
        gui_controller.RenderFontsWithFreetype();

        Graphics::Blending::Enable();
        Graphics::Blending::FuncNormalPre();
    }

    auto Resize = [&]
    {
        adaptive_viewport.Update();
        mouse.SetMatrix(adaptive_viewport.GetDetails().MouseMatrixCentered());
    };
    Resize();

    States::current_state = Poly::derived<States::Game>;

    Metronome metronome(60);
    Clock::DeltaTimer delta_timer;

    while (1)
    {
        uint64_t delta = delta_timer();
        while (metronome.Tick(delta))
        {
            window.ProcessEvents({gui_controller.EventHook()});

            if (window.Resized())
            {
                Resize();
                Graphics::Viewport(window.Size());
            }
            if (window.ExitRequested())
                Program::Exit();

            gui_controller.PreTick();
            States::current_state->Tick();
        }

        gui_controller.PreRender();
        adaptive_viewport.BeginFrame();
        States::current_state->Render();
        adaptive_viewport.FinishFrame();
        Graphics::CheckErrors();
        gui_controller.PostRender();

        window.SwapBuffers();
    }
}
