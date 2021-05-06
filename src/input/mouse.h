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
        mutable bool no_delta_next_tick = true;
        fmat3 matrix = fmat3();

        void Update() const
        {
            // Get window.
            auto window = Interface::Window::Get();

            // Stop if the mouse was already updated this tick.
            uint64_t tick = window.Ticks();
            if (update_time == tick)
                return;

            // Remember old cur_pos.
            fvec2 prev_pos_f = cur_pos_f;
            ivec2 prev_pos = cur_pos;

            // Get cur_pos and transform it.
            cur_pos_f = (matrix * window.MousePos().to_vec3(1)).to_vec2();
            cur_pos = iround(cur_pos_f);

            // Get movement.
            cur_movement = window.MouseMovement();

            // Compute delta if needed.
            if (no_delta_next_tick)
            {
                no_delta_next_tick = false;
                cur_pos_delta_f = fvec2();
                cur_pos_delta = ivec2();
            }
            else
            {
                cur_pos_delta_f = cur_pos_f - prev_pos_f;
                cur_pos_delta = cur_pos - prev_pos;
            }

            // Remember current time.
            update_time = tick;
        }

      public:
        Mouse() {}
        Mouse(const fmat3 &mat)
        {
            SetMatrix(mat);
        }

        [[nodiscard]] ivec2 pos() const
        {
            Update();
            return cur_pos;
        }
        [[nodiscard]] ivec2 pos_delta() const
        {
            Update();
            return cur_pos_delta;
        }
        [[nodiscard]] fvec2 pos_f() const
        {
            Update();
            return cur_pos_f;
        }
        [[nodiscard]] fvec2 pos_delta_f() const
        {
            Update();
            return cur_pos_delta_f;
        }
        [[nodiscard]] ivec2 movement() const
        {
            Update();
            return cur_movement;
        }

        Button left   = Button(mouse_left);
        Button middle = Button(mouse_middle);
        Button right  = Button(mouse_right);
        Button x1     = Button(mouse_x1);
        Button x2     = Button(mouse_x2);
        Button wheel_up    = Button(mouse_wheel_up);
        Button wheel_down  = Button(mouse_wheel_down);
        Button wheel_left  = Button(mouse_wheel_left);
        Button wheel_right = Button(mouse_wheel_right);

        void HideCursor(bool hide = true)
        {
            Interface::Window::Get().HideCursor(hide);
        }

        void RelativeMouseMode(bool relative = true)
        {
            Interface::Window::Get().RelativeMouseMode(relative);
        }

        [[nodiscard]] const fmat3 &Matrix() const
        {
            return matrix;
        }
        void SetMatrix(const fmat3 &mat)
        {
            matrix = mat;
            no_delta_next_tick = true;
        }
    };
}
