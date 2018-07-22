#ifndef ERRORS_H_INCLUDED
#define ERRORS_H_INCLUDED

#include <string>

#include "exit.h"
#include "messagebox.h"
#include "strings.h"

namespace Program
{
    [[noreturn]] inline void HardError(const std::string &message)
    {
        static bool first = 1;
        if (!first)
            Exit(1);
        first = 0;

        Interface::MessageBox(Interface::MessageBoxType::error, "Error", message.c_str());
        Exit(1);
    }

    [[noreturn]] inline void Error(const std::string &message) // Throws std::runtime_error.
    {
        throw std::runtime_error(message);
    }

    template <typename ...P> [[noreturn]] void HardError(const P &... params)
    {
        HardError(Str(params...));
    }

    template <typename ...P> [[noreturn]] void Error(const P &... params)
    {
        Error(Str(params...));
    }
}

#endif
