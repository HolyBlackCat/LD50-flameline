#pragma once

#include <string>

namespace Interface
{
    enum class MessageBoxType {info, warning, error};

    void MessageBox(std::string title, std::string message);
    void MessageBox(MessageBoxType type, std::string title, std::string message);
}
