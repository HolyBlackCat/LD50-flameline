#include "messagebox.h"

#include <iostream>

#include <SDL.h>

#include "interface/window.h"

namespace Interface
{
    static int GetFlagsForMessageBoxType(MessageBoxType type)
    {
        switch (type)
        {
          case MessageBoxType::info:
          default:
            return SDL_MESSAGEBOX_INFORMATION;
            break;
          case MessageBoxType::warning:
            return SDL_MESSAGEBOX_WARNING;
            break;
          case MessageBoxType::error:
            return SDL_MESSAGEBOX_ERROR;
            break;
        }
    }

    void MessageBox(const std::string &title, const std::string &message)
    {
        MessageBox(MessageBoxType::info, title, message);
    }

    void MessageBox(MessageBoxType type, const std::string &title, const std::string &message)
    {
        int flags = GetFlagsForMessageBoxType(type);

        if (SDL_ShowSimpleMessageBox(flags, title.c_str(), message.c_str(), Window::GetHandleOrNull()))
        {
            std::cerr << "Unable to show a message box!\n" << title << '\n' << message << '\n';
        }
    }

    int MessageBox(const std::string &title, const std::string &message, const std::vector<std::string> &buttons)
    {
        return MessageBox(MessageBoxType::info, title, message, buttons);
    }

    int MessageBox(MessageBoxType type, const std::string &title, const std::string &message, const std::vector<std::string> &buttons)
    {
        // SDL2 supports selecting a messagebox button by default, but this functionality
        // doesn't seem to work properly at least on Windows, so I'm not exposing it.

        if (buttons.size() == 0)
        {
            MessageBox(type, title, message);
            return 0;
        }

        SDL_MessageBoxData data{};
        data.flags = GetFlagsForMessageBoxType(type);
        data.title = title.c_str();
        data.message = message.c_str();
        data.window = Window::GetHandleOrNull();
        data.numbuttons = buttons.size();

        std::vector<SDL_MessageBoxButtonData> button_data;
        button_data.reserve(buttons.size());
        int button_index = 0;
        for (const auto &button_name : buttons)
        {
            auto &this_button = button_data.emplace_back();
            this_button.text = button_name.c_str();
            this_button.buttonid = button_index++;
        }
        data.buttons = button_data.data();

        int result = -1;
        if (SDL_ShowMessageBox(&data, &result))
        {
            std::cerr << "Unable to show a message box!\n" << title << '\n' << message << '\n';

            for (const auto &button_name : buttons)
                std::cerr << " [" << button_name << "]";
            std::cerr << '\n';

            return -1;
        }

        return result;
    }
}
