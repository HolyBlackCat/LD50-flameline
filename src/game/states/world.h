#include "game/meta.h"

#include "graphics/text.h"

namespace States
{
    class World : public State
    {
        float angle = 0;
      public:
        void Tick() override
        {
            angle += 0.01;
        }

        void Render() const override
        {
            viewport.BeginFrame();
            Graphics::Clear();
            render.BindShader();

            //render.iquad(ivec2(0), ivec2(32)).color(fvec3(0,0.5,1)).center().rotate(angle);
            render.ftriangle(fvec2::dir(angle + f_pi*2/3, 64), fvec2::dir(angle, 64), fvec2::dir(angle - f_pi*2/3, 64)).color(fvec3(1,0,0), fvec3(0,1,0), fvec3(0,0,1));

            render.itext(controls.mouse.pos(), {font_main, "Hello!\nПривет!\n123"}).color(fvec3(1,1,0.75)).align(ivec2(-1,0)).align_box_x(0);

            render.Finish();
            viewport.FinishFrame();
            Graphics::CheckErrors();
        }
    };
}
