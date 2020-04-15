const ivec2 screen_size(480, 270);
Interface::Window window("Iota", screen_size * 2, Interface::windowed, adjust_(Interface::WindowSettings{}, min_size = screen_size));
Graphics::DummyVertexArray dummy_vao = nullptr;

const Graphics::ShaderConfig shader_config = Graphics::ShaderConfig::Core();
Interface::ImGuiController gui_controller(Poly::derived<Interface::ImGuiController::GraphicsBackend_Modern>, adjust_(Interface::ImGuiController::Config{}, shader_header = shader_config.common_header));

Graphics::TextureAtlas texture_atlas(ivec2(2048), "assets/_images", "assets/atlas.png", "assets/atlas.refl");
Graphics::Texture texture_main = Graphics::Texture(nullptr).Wrap(Graphics::clamp).Interpolation(Graphics::nearest).SetData(texture_atlas.GetImage());

AdaptiveViewport adaptive_viewport(shader_config, screen_size);
Render r = adjust_(Render(0x2000, shader_config), SetTexture(texture_main), SetMatrix(adaptive_viewport.GetDetails().MatrixCentered()));

Input::Mouse mouse;

struct State : Program::DefaultBasicState
{
    void Resize()
    {
        adaptive_viewport.Update();
        mouse.SetMatrix(adaptive_viewport.GetDetails().MouseMatrixCentered());
    }

    int last_second = -1;
    int tick_counter = 0, frame_counter = 0;
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
        int cur_second = SDL_GetTicks() / 1000;
        if (cur_second == last_second)
            return;

        last_second = cur_second;
        std::cout << "TPS: " << tick_counter << "\n";
        std::cout << "FPS: " << frame_counter << "\n\n";
        tick_counter = 0;
        frame_counter = 0;
    }

    void Tick() override
    {
        tick_counter++;

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
        HighLevelTick();
    }

    void Render() override
    {
        frame_counter++;

        gui_controller.PreRender();
        adaptive_viewport.BeginFrame();
        HighLevelRender();
        adaptive_viewport.FinishFrame();
        Graphics::CheckErrors();
        gui_controller.PostRender();

        window.SwapBuffers();
    }


    void Init()
    {
        ImGui::StyleColorsDark();

        // Load various small fonts
        auto monochrome_font_flags = ImGuiFreeType::Monochrome | ImGuiFreeType::MonoHinting;

        gui_controller.LoadFont("assets/Monokat_6x12.ttf", 12.0f, adjust(ImFontConfig{}, RasterizerFlags = monochrome_font_flags));
        gui_controller.LoadDefaultFont();
        gui_controller.RenderFontsWithFreetype();

        Graphics::Blending::Enable();
        Graphics::Blending::FuncNormalPre();
    }

    float angle = 0;

    void HighLevelTick()
    {
        angle += 0.01;
        ImGui::ShowDemoWindow();
    }

    void HighLevelRender()
    {
        Graphics::SetClearColor(fvec3(0));
        Graphics::Clear();

        r.BindShader();

        r.iquad(mouse.pos(), ivec2(32)).center().rotate(angle).color(mouse.left.down() ? fvec3(1,0.5,0) : fvec3(0,0.5,1));

        r.Finish();
    }
};

int _main_(int, char **)
{
    State loop_state;
    loop_state.Init();
    loop_state.Resize();
    loop_state.RunMainLoop();
    return 0;
}
