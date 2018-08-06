#ifndef INTERFACE_MESSAGEBOX_H_INCLUDED
#define INTERFACE_MESSAGEBOX_H_INCLUDED

#include <string>

namespace Interface
{
    enum class MessageBoxType {info, warning, error};

    void MessageBox(std::string title, std::string message);
    void MessageBox(MessageBoxType type, std::string title, std::string message);
}

#endif
