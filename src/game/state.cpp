#include "game/main.h"
#include "reflection/short_macros.h"

namespace States
{
    STRUCT( Initial EXTENDS State::BasicState )
    {
        UNNAMED_MEMBERS()

        float angle = 0;

        void Tick(const State::NextStateSelector &next_state) override
        {
            (void)next_state;

            angle += 0.01;
            ImGui::ShowDemoWindow();
        }

        void Render() const override
        {
            Graphics::SetClearColor(fvec3(0));
            Graphics::Clear();

            r.BindShader();

            r.iquad(mouse.pos(), ivec2(32)).center().rotate(angle).color(mouse.left.down() ? fvec3(1,0.5,0) : fvec3(0,0.5,1));

            r.Finish();
        }
    };
}
