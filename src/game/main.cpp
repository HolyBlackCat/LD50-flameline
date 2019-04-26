
int main()
{
    ivec2 window_size(800, 600);

    Interface::WindowSettings window_settings;
    window_settings.min_size = window_size / 2;

    Interface::Window window("Delta", window_size, Interface::windowed, window_settings);

    Graphics::ShaderConfig shader_config = Graphics::ShaderConfig::Core();
    Interface::ImGuiController gui_controller(shader_config.common_header);
    ImGui::StyleColorsDark();

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
                Graphics::Viewport(window.Size());
                std::cout << "Resized.\n";
            }
            if (window.ExitRequested())
                Program::Exit();

            gui_controller.PreTick();
            // Tick
            ImGui::ShowDemoWindow();
        }

        Graphics::Clear();
        gui_controller.PreRender();
        // Render
        gui_controller.PostRender();

        window.SwapBuffers();
    }
}
