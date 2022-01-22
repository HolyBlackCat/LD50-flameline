#include "main.h"

namespace States
{
    STRUCT( World EXTENDS StateBase )
    {
        MEMBERS()

        float angle = 0;

        void Tick(std::string &next_state) override
        {
            (void)next_state;

            angle += 0.01;
            ImGui::ShowDemoWindow();

            if (mouse.right.pressed())
            {
                static Audio::Buffer buf = []{
                    int16_t array[10000];
                    for (size_t i = 0; i < std::size(array); i++)
                        array[i] = std::sin(i / 30.f) * 0x7fff;
                    return Audio::Sound(48000, Audio::mono, std::size(array), array);
                }();
                audio_controller.Add(buf)->play();
            }
        }

        void Render() const override
        {
            Graphics::SetClearColor(fvec3(0));
            Graphics::Clear();

            r.BindShader();

            r.iquad(mouse.pos(), ivec2(32)).center().rotate(angle).color(mouse.left.down() ? fvec3(1,0.5,0) : fvec3(0,0.5,1));
            r.itext(mouse.pos(), Graphics::Text(Fonts::main, STR((audio_controller.ActiveSources())))).color(fvec3(1));

            r.Finish();
        }
    };
}
