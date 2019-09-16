#pragma once

#include <string>
#include <vector>

namespace Interface
{
    enum class MessageBoxType {info, warning, error};

    void MessageBox(const std::string &title, const std::string &message);
    void MessageBox(MessageBoxType type, const std::string &title, const std::string &message);

    // Return an index of the pressed button, or -1 on failure.
    int MessageBox(const std::string &title, const std::string &message, const std::vector<std::string> &buttons);
    int MessageBox(MessageBoxType type, const std::string &title, const std::string &message, const std::vector<std::string> &buttons);
}
