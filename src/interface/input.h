#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <initializer_list>
#include <vector>

#include "window.h"

namespace Interface
{
    class Button
    {
        Inputs::Enum index = Inputs::None;

        mutable uint64_t update_time = 0;
        mutable bool is_pressed = 0, is_released = 0, is_down = 0, is_repeated = 0;

        void Update() const
        {
            auto &window = Window::Instance();

            uint64_t tick = window.Ticks();
            if (update_time == tick)
                return;
            update_time = tick;

            auto times = window.InputTimes(index);

            is_pressed = times.press == tick;
            is_released = times.release == tick;
            is_down = times.press > times.release;
            is_repeated = times.repeat == tick;
        }

        bool Assign(Inputs::Enum begin, Inputs::Enum end)
        {
            auto &window = Window::Instance();
            uint64_t tick = window.Ticks();

            for (auto i = begin; i < end; i = Inputs::Enum(i+1))
            {
                if (window.InputTimes(i).press == tick)
                {
                    index = i;
                    return 1;
                }
            }

            return 0;
        }

      public:
        Button() {}
        Button(Inputs::Enum index) : index(index) {}

        [[nodiscard]] bool pressed () const {Update(); return is_pressed;}
        [[nodiscard]] bool released() const {Update(); return is_released;}
        [[nodiscard]] bool repeated() const {Update(); return is_repeated;}
        [[nodiscard]] bool down    () const {Update(); return is_down;}
        [[nodiscard]] bool up      () const {Update(); return !is_down;}

        explicit operator bool() const {return index != Inputs::None;}

        Inputs::Enum Index() const {return index;}

        bool AssignKey()
        {
            return Assign(Inputs::BeginKeys, Inputs::EndKeys);
        }
        bool AssignMouseButton()
        {
            return Assign(Inputs::BeginMouseButtons, Inputs::EndMouseButtons);
        }
        bool AssignMouseWheel()
        {
            return Assign(Inputs::BeginMouseWheel, Inputs::EndMouseWheel);
        }
    };
}

#endif
