#pragma once

#include <string>

#include "input/enum.h"
#include "interface/window.h"

namespace Input
{
    class Button
    {
        Enum index = None;

        mutable uint64_t update_time = 0;
        mutable bool is_pressed = 0, is_released = 0, is_down = 0, is_repeated = 0;

        void Update() const
        {
            auto &window = Interface::Window::Get();

            uint64_t tick = window.Ticks();
            if (update_time == tick)
                return;

            auto times = window.GetInputTimes(index);
            is_pressed = times.press == tick;
            is_released = times.release == tick;
            is_down = times.press > times.release;
            is_repeated = times.repeat == tick;

            update_time = tick;
        }

        bool Assign(Enum begin, Enum end)
        {
            auto &window = Interface::Window::Get();
            uint64_t tick = window.Ticks();

            for (auto i = begin; i < end; i = Enum(i+1))
            {
                if (window.GetInputTimes(i).press == tick)
                {
                    index = i;
                    return 1;
                }
            }

            return 0;
        }

      public:
        Button() {}
        Button(Enum index) : index(index) {}

        [[nodiscard]] bool pressed () const {Update(); return is_pressed;}
        [[nodiscard]] bool released() const {Update(); return is_released;}
        [[nodiscard]] bool repeated() const {Update(); return is_repeated;}
        [[nodiscard]] bool down    () const {Update(); return is_down;}
        [[nodiscard]] bool up      () const {Update(); return !is_down;}

        [[nodiscard]] explicit operator bool() const {return index != None;}

        [[nodiscard]] Enum Index() const
        {
            return index;
        }

        [[nodiscard]] std::string Name() const // Returns a layout-dependent name, which
        {
            if (index == None)
            {
                return "None";
            }
            else if (index >= BeginKeys && index < EndKeys)
            {
                const char *ret;

                if (SDL_Keycode keycode = SDL_GetKeyFromScancode(SDL_Scancode(index)))
                    ret = SDL_GetKeyName(keycode);
                else
                    ret = SDL_GetScancodeName(SDL_Scancode(index));

                if (*ret) // We don't need to check for null pointers, since above functions never return those.
                    return ret;
                else
                    return "Unknown " + std::to_string(index);
            }
            else if (index >= BeginMouseButtons && index < EndMouseButtons)
            {
                switch (index)
                {
                  case mouse_left:
                    return "Left Mouse Button";
                  case mouse_middle:
                    return "Middle Mouse Button";
                  case mouse_right:
                    return "Right Mouse Button";
                  case mouse_x1:
                    return "X1 Mouse Button";
                  case mouse_x2:
                    return "X2 Mouse Button";
                  default:
                    return "Mouse Button " + std::to_string(index - mouse_left + 1);
                }
            }
            else
            {
                switch (index)
                {
                  case mouse_wheel_up:
                    return "Mouse Wheel Up";
                  case mouse_wheel_down:
                    return "Mouse Wheel Down";
                  case mouse_wheel_left:
                    return "Mouse Wheel Left";
                  case mouse_wheel_right:
                    return "Mouse Wheel Right";
                  default:
                    return "Invalid " + std::to_string(index);
                }
            }
        }

        bool AssignKey()
        {
            return Assign(BeginKeys, EndKeys);
        }
        bool AssignMouseButton()
        {
            return Assign(BeginMouseButtons, EndMouseButtons);
        }
        bool AssignMouseWheel()
        {
            return Assign(BeginMouseWheel, EndMouseWheel);
        }
    };
}
