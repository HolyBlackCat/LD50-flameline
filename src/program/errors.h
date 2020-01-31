#pragma once

#include <exception>
#include <string>

#include "interface/messagebox.h"
#include "program/exit.h"
#include "strings/common.h"

namespace Program
{
    [[noreturn]] inline void HardError(const std::string &message)
    {
        static bool first = 1;
        if (!first)
            Exit(1);
        first = 0;

        Interface::MessageBox(Interface::MessageBoxType::error, "Error", "Error: " + message);
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

    void SetErrorHandlers(bool only_if_not_set_before = 1);

    #ifdef NDEBUG
    #  define DebugAssert(text, ...) do {} while (0)
    #  define DebugAssertNameless(...) do {} while (0)
    #else
    #  define DebugAssert(text, ...) do {bool(__VA_ARGS__) || (::Program::HardError("Assertion failed: ", text), 0);} while (0)
    #  define DebugAssertNameless(...) do {bool(__VA_ARGS__) || (::Program::HardError("Assertion failed.\nExpression: " #__VA_ARGS__), 0);} while (0)
    #endif
}
