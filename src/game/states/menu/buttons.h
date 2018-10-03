#pragma once

namespace States::Details::Menu
{
    class Buttons
    {
      public:
        static constexpr int button_count = 2;

      private:
        struct Button
        {
            float time = 0;
        };
        Button buttons[button_count];

        int active_button = 0;

      public:
        Buttons();

        void Tick();
        void Render() const;
    };
}
