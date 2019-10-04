const ivec2 screen_size(480, 270);
Interface::Window window("Iota", screen_size * 2, Interface::windowed, adjust_(Interface::WindowSettings{}, min_size = screen_size));
Graphics::DummyVertexArray dummy_vao = nullptr;

const Graphics::ShaderConfig shader_config = Graphics::ShaderConfig::Core();
Interface::ImGuiController gui_controller(Poly::derived<Interface::ImGuiController::GraphicsBackend_Modern>, adjust_(Interface::ImGuiController::Config{}, shader_header = shader_config.common_header));

Graphics::TextureAtlas texture_atlas(ivec2(2048), "assets/_images", "assets/atlas.png", "assets/atlas.refl");
Graphics::Texture texture_main = Graphics::Texture(nullptr).Wrap(Graphics::clamp).Interpolation(Graphics::nearest).SetData(texture_atlas.GetImage());

// AdaptiveViewport adaptive_viewport(shader_config, screen_size);
// Render r = adjust_(Render(0x2000, shader_config), SetTexture(texture_main), SetMatrix(adaptive_viewport.GetDetails().MatrixCentered()));

using Renderer = Graphics::Renderers::Flat;
Renderer r(shader_config, 2, 4);

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
        float angle = 0;

        Game()
        {
            r.SetBlendingMode();
            r.SetTexture(texture_main);
        }

        void Tick() override
        {
            angle += 0.01;
            ImGui::ShowDemoWindow();
        }

        void Render() const override
        {
            Graphics::SetClearColor(fvec3(0));
            Graphics::Clear();

            r << r.translate(mouse.pos()) * r.TexturedQuad(texture_atlas.Get("dummy.png"));

            for (int i = 0; i < 10; i++)
            {
                float a = i * f_pi / 5;
                r << r.translate(mouse.pos() + fvec2::dir(a, 100)) * r.rotate(-angle/2) * r.UntexturedQuad(fvec2(32)).CenterRel(fvec2(0.5)).Color(fmat3::rotate(fvec3(1), angle + a) * fvec3(1,0,0));
            }

            r << r.translate(mouse.pos()) * r.rotate(angle) * r.UntexturedQuad(fvec2(32)).Centered().Color(fvec3(1,0,0));

            r.Flush();

            // r.BindShader();

            // r.iquad(mouse.pos(), ivec2(32)).center().rotate(window.Ticks() / 100.).color(mouse.left.down() ? fvec3(1,0.5,0) : fvec3(0,0.5,1));

            // r.Finish();
        }
    };
}

int ENTRY_POINT(int, char **)
{
    { // Initialize
        ImGui::StyleColorsDark();

        // Load various small fonts
        auto monochrome_font_flags = ImGuiFreeType::Monochrome | ImGuiFreeType::MonoHinting;

        gui_controller.LoadFont("assets/Monokat_6x12.ttf", 12.0f, adjust(ImFontConfig{}, RasterizerFlags = monochrome_font_flags));
        gui_controller.LoadDefaultFont();
        gui_controller.RenderFontsWithFreetype();

        Graphics::Blending::Enable();
        Graphics::Blending::FuncNormalPre();
    }

    auto Resize = [&]
    {
        ivec2 a = window.Size() / -2, b = a + window.Size();

        r.SetMatrix(fmat4::ortho(ivec2(a.x, b.y), ivec2(b.x, a.y), -1, 1));
        mouse.SetMatrix(fmat3::translate(a));

        // adaptive_viewport.Update();
        // mouse.SetMatrix(adaptive_viewport.GetDetails().MouseMatrixCentered());
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
            // window.ProcessEvents();
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
        // adaptive_viewport.BeginFrame();
        States::current_state->Render();
        // adaptive_viewport.FinishFrame();
        Graphics::CheckErrors();
        gui_controller.PostRender();

        window.SwapBuffers();
    }
}
