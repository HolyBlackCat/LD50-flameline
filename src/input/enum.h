#pragma once

#include <SDL_scancode.h>

namespace Input
{
    enum Enum
    {
        // No input
        None = 0,

        // Keys
        BeginKeys = 0, // It's guaranteed that key 0 is never used.

        _0 = SDL_SCANCODE_0,
        _1 = SDL_SCANCODE_1,
        _2 = SDL_SCANCODE_2,
        _3 = SDL_SCANCODE_3,
        _4 = SDL_SCANCODE_4,
        _5 = SDL_SCANCODE_5,
        _6 = SDL_SCANCODE_6,
        _7 = SDL_SCANCODE_7,
        _8 = SDL_SCANCODE_8,
        _9 = SDL_SCANCODE_9,

        a = SDL_SCANCODE_A,
        b = SDL_SCANCODE_B,
        c = SDL_SCANCODE_C,
        d = SDL_SCANCODE_D,
        e = SDL_SCANCODE_E,
        f = SDL_SCANCODE_F,
        g = SDL_SCANCODE_G,
        h = SDL_SCANCODE_H,
        i = SDL_SCANCODE_I,
        j = SDL_SCANCODE_J,
        k = SDL_SCANCODE_K,
        l = SDL_SCANCODE_L,
        m = SDL_SCANCODE_M,
        n = SDL_SCANCODE_N,
        o = SDL_SCANCODE_O,
        p = SDL_SCANCODE_P,
        q = SDL_SCANCODE_Q,
        r = SDL_SCANCODE_R,
        s = SDL_SCANCODE_S,
        t = SDL_SCANCODE_T,
        u = SDL_SCANCODE_U,
        v = SDL_SCANCODE_V,
        w = SDL_SCANCODE_W,
        x = SDL_SCANCODE_X,
        y = SDL_SCANCODE_Y,
        z = SDL_SCANCODE_Z,

        apostrophe = SDL_SCANCODE_APOSTROPHE,
        comma      = SDL_SCANCODE_COMMA,
        period     = SDL_SCANCODE_PERIOD,
        minus      = SDL_SCANCODE_MINUS,
        equals     = SDL_SCANCODE_EQUALS,
        semicolon  = SDL_SCANCODE_SEMICOLON,
        slash      = SDL_SCANCODE_SLASH,
        l_bracket  = SDL_SCANCODE_LEFTBRACKET,
        r_bracket  = SDL_SCANCODE_RIGHTBRACKET,
        backslash  = SDL_SCANCODE_BACKSLASH,
        grave      = SDL_SCANCODE_GRAVE,

        num_0 = SDL_SCANCODE_KP_0,
        num_1 = SDL_SCANCODE_KP_1,
        num_2 = SDL_SCANCODE_KP_2,
        num_3 = SDL_SCANCODE_KP_3,
        num_4 = SDL_SCANCODE_KP_4,
        num_5 = SDL_SCANCODE_KP_5,
        num_6 = SDL_SCANCODE_KP_6,
        num_7 = SDL_SCANCODE_KP_7,
        num_8 = SDL_SCANCODE_KP_8,
        num_9 = SDL_SCANCODE_KP_9,

        num_period    = SDL_SCANCODE_KP_PERIOD,
        num_equals    = SDL_SCANCODE_KP_EQUALS,
        num_enter     = SDL_SCANCODE_KP_ENTER,
        num_plus      = SDL_SCANCODE_KP_PLUS,
        num_minus     = SDL_SCANCODE_KP_MINUS,
        num_multiply  = SDL_SCANCODE_KP_MULTIPLY,
        num_divide    = SDL_SCANCODE_KP_DIVIDE,

        f1  = SDL_SCANCODE_F1,
        f2  = SDL_SCANCODE_F2,
        f3  = SDL_SCANCODE_F3,
        f4  = SDL_SCANCODE_F4,
        f5  = SDL_SCANCODE_F5,
        f6  = SDL_SCANCODE_F6,
        f7  = SDL_SCANCODE_F7,
        f8  = SDL_SCANCODE_F8,
        f9  = SDL_SCANCODE_F9,
        f10 = SDL_SCANCODE_F10,
        f11 = SDL_SCANCODE_F11,
        f12 = SDL_SCANCODE_F12,

        application  = SDL_SCANCODE_APPLICATION,
        menu         = SDL_SCANCODE_MENU,
        escape       = SDL_SCANCODE_ESCAPE,
        enter        = SDL_SCANCODE_RETURN,
        tab          = SDL_SCANCODE_TAB,
        backspace    = SDL_SCANCODE_BACKSPACE,
        insert       = SDL_SCANCODE_INSERT,
        del          = SDL_SCANCODE_DELETE,
        up           = SDL_SCANCODE_UP,
        down         = SDL_SCANCODE_DOWN,
        left         = SDL_SCANCODE_LEFT,
        right        = SDL_SCANCODE_RIGHT,
        page_up      = SDL_SCANCODE_PAGEUP,
        page_down    = SDL_SCANCODE_PAGEDOWN,
        home         = SDL_SCANCODE_HOME,
        end          = SDL_SCANCODE_END,
        caps_lock    = SDL_SCANCODE_CAPSLOCK,
        scroll_lock  = SDL_SCANCODE_SCROLLLOCK,
        num_lock     = SDL_SCANCODE_NUMLOCKCLEAR,
        print_screen = SDL_SCANCODE_PRINTSCREEN,
        pause        = SDL_SCANCODE_PAUSE,
        space        = SDL_SCANCODE_SPACE,

        l_shift      = SDL_SCANCODE_LSHIFT,
        r_shift      = SDL_SCANCODE_RSHIFT,
        l_ctrl       = SDL_SCANCODE_LCTRL,
        r_ctrl       = SDL_SCANCODE_RCTRL,
        l_alt        = SDL_SCANCODE_LALT,
        r_alt        = SDL_SCANCODE_RALT,
        l_gui        = SDL_SCANCODE_LGUI,
        r_gui        = SDL_SCANCODE_RGUI,

        EndKeys = SDL_NUM_SCANCODES,

        // Mouse buttons
        BeginMouseButtons = EndKeys,

        mouse_left = BeginMouseButtons,
        mouse_middle,
        mouse_right,
        mouse_x1,
        mouse_x2,

        EndMouseButtons = BeginMouseButtons + 32,

        // Mouse wheen
        BeginMouseWheel = EndMouseButtons,

        mouse_wheel_up = BeginMouseWheel,
        mouse_wheel_down,
        mouse_wheel_left,
        mouse_wheel_right,

        EndMouseWheel = BeginMouseWheel + 4,

        // Index count
        IndexCount = EndMouseWheel,
    };
}
