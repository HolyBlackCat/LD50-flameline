#include "messagebox.h"

#include <SDL2/SDL.h>

#include "window.h"

namespace Interface
{
    void MessageBox(std::string title, std::string message)
    {
        MessageBox(MessageBoxType::info, title, message);
    }

    void MessageBox(MessageBoxType type, std::string title, std::string message)
    {
        int type_value;
        switch (type)
        {
          case MessageBoxType::info:
          default:
            type_value = SDL_MESSAGEBOX_INFORMATION;
            break;
          case MessageBoxType::warning:
            type_value = SDL_MESSAGEBOX_WARNING;
            break;
          case MessageBoxType::error:
            type_value = SDL_MESSAGEBOX_ERROR;
            break;
        }

        SDL_ShowSimpleMessageBox(type_value, title.c_str(), message.c_str(), Window::HandleOrNull());
    }
}
