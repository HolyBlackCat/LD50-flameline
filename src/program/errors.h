#pragma once

#include <stdexcept>
#include <string_view>
#include <string>

#include "interface/messagebox.h"
#include "program/exit.h"
#include "strings/format.h"

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

    template <typename ...P>
    [[noreturn]] void HardError(const P &... params)
    {
        HardError(Strings::Concat(params...));
    }

    template <typename ...P>
    [[noreturn]] void Error(const P &... params)
    {
        Error(Strings::Concat(params...));
    }

    void SetErrorHandlers(bool only_if_not_set_before = 1);


    namespace impl
    {
        // An assertion function.
        // Triggers an error if `enable && !condition`.
        inline bool Assert(bool enable, bool condition, std::string_view message_or_expr, const char *expr_or_nothing = nullptr)
        {
            if (!enable || condition)
                return true;

            if (expr_or_nothing)
            {
                // User specified a custom message.
                HardError("Assertion failed:\n", message_or_expr);
            }
            else
            {
                // User didn't specify a message.
                HardError("Assertion failed for expression:\n", message_or_expr);
            }

            return condition;
        }

        // A template overload that allows using explicitly-but-not-implicitly-convertible-to-bool expressions as conditions.
        template <typename T>
        bool Assert(bool enable, const T &condition, std::string_view message_or_expr, const char *expr_or_nothing = nullptr)
        {
            return Assert(enable, bool(condition), message_or_expr, expr_or_nothing);
        }
    }
}


// A debug assertion.
// Can be called either as `ASSERT(bool)` or `ASSERT(bool, std::string_view)`.
// In the first case, the expression is printed in the message, otherwise the custom message is printed.
#define ASSERT(...) ASSERT_impl(__VA_ARGS__)
#define ASSERT_impl(...) ::Program::impl::Assert(IMP_RE_ENABLE_ASSERTIONS, __VA_ARGS__, #__VA_ARGS__)

// A macro controlling the assertions.
// You can redefine it at any time to change the behavior or ASSERT.
#ifndef IMP_RE_ENABLE_ASSERTIONS
#  define IMP_RE_ENABLE_ASSERTIONS IMP_RE_ENABLE_ASSERTIONS_DEFAULT
#endif

// The default value of `IMP_RE_ENABLE_ASSERTIONS`.
#ifdef NDEBUG
#  define IMP_RE_ENABLE_ASSERTIONS_DEFAULT 0
#else
#  define IMP_RE_ENABLE_ASSERTIONS_DEFAULT 1
#endif
