#pragma once

#include "enum.h"
#include "interface/window.h"
#include "utils/mat.h"

namespace Input
{
    class Mouse
    {
        mutable uint64_t update_time = 0;
        mutable ivec2 cur_pos = ivec2(0), cur_pos_delta = ivec2(0), cur_movement = ivec2(0);
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
            ivec2 prev_position = cur_pos;

            // Get cur_pos and transform it
            cur_pos = iround((matrix * window.MousePos().to_vec3(1)).to_vec2());

            // Get movement
            cur_movement = window.MouseMovement();

            // Compute delta if needed
            if (no_delta_next_tick == 0)
                cur_pos_delta = cur_pos - prev_position;
            else
                no_delta_next_tick = 0;

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
