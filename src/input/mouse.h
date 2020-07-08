#pragma once

#include "input/button.h"
#include "input/enum.h"
#include "interface/window.h"
#include "utils/mat.h"

namespace Input
{
    class Mouse
    {
        mutable uint64_t update_time = 0;
        mutable fvec2 cur_pos_f = fvec2(0), cur_pos_delta_f = fvec2(0);
        mutable ivec2 cur_pos = ivec2(0), cur_pos_delta = ivec2(0);
        mutable ivec2 cur_movement = ivec2(0);
        mutable bool no_delta_next_tick = 1;
        fmat3 matrix = fmat3();

        void Update() const
        {
            // Get window
            auto &window = Interface::Window::Get();

            // Stop if the mouse was already updated this tick.
            uint64_t tick = window.Ticks();
            if (update_time == tick)
                return;

            // Remember old cur_pos
            fvec2 prev_pos_f = cur_pos_f;
            ivec2 prev_pos = cur_pos;

            // Get cur_pos and transform it
            cur_pos_f = (matrix * window.MousePos().to_vec3(1)).to_vec2();
            cur_pos = iround(cur_pos_f);

            // Get movement
            cur_movement = window.MouseMovement();

            // Compute delta if needed
            if (no_delta_next_tick == 0)
            {
                cur_pos_delta_f = cur_pos_f - prev_pos_f;
                cur_pos_delta = cur_pos - prev_pos;
            }
            else
            {
                no_delta_next_tick = 0;
            }

            // Remember current time
            update_time = tick;
        }

      public:
        Mouse() {}
        Mouse(const fmat3 &mat)
        {
            SetMatrix(mat);
        }

        ivec2 pos() const
        {
            Update();
            return cur_pos;
        }
        ivec2 pos_delta() const
        {
            Update();
            return cur_pos_delta;
        }
        fvec2 pos_f() const
        {
            Update();
            return cur_pos_f;
        }
        fvec2 pos_delta_f() const
        {
            Update();
            return cur_pos_delta_f;
        }
        ivec2 movement() const
        {
            Update();
            return cur_movement;
        }

        Button left   = Button(Input::mouse_left);
        Button middle = Button(Input::mouse_middle);
        Button right  = Button(Input::mouse_right);
        Button x1     = Button(Input::mouse_x1);
        Button x2     = Button(Input::mouse_x2);
        Button wheel_up    = Button(Input::mouse_wheel_up);
        Button wheel_down  = Button(Input::mouse_wheel_down);
        Button wheel_left  = Button(Input::mouse_wheel_left);
        Button wheel_right = Button(Input::mouse_wheel_right);

        void HideCursor(bool hide = 1)
        {
            Interface::Window::Get().HideCursor(hide);
        }

        void RelativeMouseMode(bool relative = 1)
        {
            Interface::Window::Get().RelativeMouseMode(relative);
        }

        const fmat3 &Matrix() const
        {
            return matrix;
        }
        void SetMatrix(const fmat3 &mat)
        {
            matrix = mat;
            no_delta_next_tick = 1;
        }
    };
}
