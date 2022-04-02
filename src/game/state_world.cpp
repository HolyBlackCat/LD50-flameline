#include "main.h"

#include "game/map.h"

namespace States
{
    STRUCT( World EXTENDS StateBase )
    {
        MEMBERS()

        Map map = Stream::ReadOnlyData(Program::ExeDir() + "map.json");

        void Tick(std::string &next_state) override
        {
            (void)next_state;
        }

        void Render() const override
        {
            Graphics::SetClearColor(fvec3(0));
            Graphics::Clear();

            r.BindShader();

            map.render(mouse.pos() * 2);

            r.Finish();
        }
    };
}
