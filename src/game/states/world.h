#include "game/meta.h"

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

            render.iquad(ivec2(0), ivec2(32)).color(fvec3(0,0.5,1)).center().rotate(angle);

            render.Finish();
            viewport.FinishFrame();
            Graphics::CheckErrors();
        }
    };
}
