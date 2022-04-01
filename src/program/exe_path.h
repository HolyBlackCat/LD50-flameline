#pragma once

#include <string>

namespace Program
{
    // Returns the location of the executable file.
    // A trailing slash is guaranteed.
    [[nodiscard]] const std::string &ExeDir();
}
