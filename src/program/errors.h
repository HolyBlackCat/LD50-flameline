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
        // Making it constexpr allows us using it in compile-time contexts (as long as the condition is true, which is exactly the point).
        inline constexpr void Assert(const char *context, const char *function, bool condition, std::string_view message_or_expr, const char *expr_or_nothing = nullptr)
        {
            if (condition)
                return;

            if (expr_or_nothing)
            {
                // User specified a custom message.
                HardError(STR("Assertion failed!\n   at   ", (context), "\n   in   ", (function), "\nMessage:\n   ", (message_or_expr)));
            }
            else
            {
                // User didn't specify a message.
                HardError(STR("Assertion failed!\n   at   ", (context), "\n   in   ", (function), "\nExpression:\n   ", (message_or_expr)));
            }

            return;
        }

        // A template overload that allows using explicitly-but-not-implicitly-convertible-to-bool expressions as conditions.
        template <typename T>
        constexpr void Assert(const char *context, const char *function, const T &condition, std::string_view message_or_expr, const char *expr_or_nothing = nullptr)
        {
            return Assert(context, function, bool(condition), message_or_expr, expr_or_nothing);
        }
    }
}


// An assertion macro that always works, even in release builds.
// Can be called either as `ASSERT_ALWAYS(bool)` or `ASSERT_ALWAYS(bool, std::string_view)`.
// In the first case, the expression is printed in the message, otherwise the custom message is printed.
#define ASSERT_ALWAYS(...) ASSERT_ALWAYS_impl(__LINE__, __VA_ARGS__)
#define ASSERT_ALWAYS_impl(line, ...) ASSERT_ALWAYS_impl_low(line, __VA_ARGS__)
#define ASSERT_ALWAYS_impl_low(line, ...) ::Program::impl::Assert(__FILE__ ":" #line, __PRETTY_FUNCTION__, __VA_ARGS__, #__VA_ARGS__)

// An assertion macro that only works in debug builds. (But you can redefine it if necessary.)
#ifdef NDEBUG
#  define ASSERT(...) void(0)
#else
#  define ASSERT(...) ASSERT_ALWAYS(__VA_ARGS__)
#endif
