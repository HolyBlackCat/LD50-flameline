#pragma once

namespace States::Details::Menu
{
    class Background
    {
        int time = 0;
      public:
        Background() {}

        void Tick();
        void Render() const;
    };
}
