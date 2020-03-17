#pragma once

// CGLFL (configurable OpenGL function loader)
//   by Egor Mikhailov <blckcat@inbox.ru>
// License: zlib

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <type_traits>

#include "config.hpp"
#include "macros.hpp"
#include <cglfl_generated/macros_public.hpp>
#include <cglfl_generated/types.hpp>

namespace cglfl
{
    using function_loader_t = std::function<void *(const char *)>;

    // Loads all function pointers with the provided loader (excluding extensions).
    // Returns `true` on success.
    bool load(function_loader_t loader, bool throw_on_failure = true);

    // Loads all function pointers with the provided loader (from a specific extension).
    // Returns `true` on success.
    bool load_extension(function_loader_t loader, std::string extension, bool throw_on_failure = true);

    // Calls `load`, then `load_extension` for all known extensions.
    bool load_all(function_loader_t loader, bool throw_on_failure = true);

    // Holds function pointers.
    struct context
    {
        void *functions[CGLFL_FUNC_COUNT] {};
        constexpr context() {}
    };

    // Points to a statically allocated context by default;
    // unless `CGLFL_NO_DEFAULT_CONTEXT` is defined in the config, in which case it's null by default.
    extern context *context_pointer;

    // Returns the default value of `context_pointer`.
    context *default_context();

    // Various debugging features.
    // Debug mode has to be enabled for those to work, see `config.hpp`.
    namespace debug
    {
        // Logs go here, one line at a time (without '\n' symbols).
        using log_func_t = std::function<void(const char *)>;
        extern log_func_t log_func; // Writes to `stdout` by default.

        // Uses `log_func` to write a string to the log.
        void log(const char *string);
        void log(const std::string &string);

        // Enables function call logging.
        extern bool log_calls; // Enabled by default.

        // When logging function calls, enables logging string parameters.
        // Disable this if you somehow got non-null-terminated strings.
        extern bool log_string_params; // Enabled by default.

        // Enables calling `glGetError` after each function call.
        extern bool check_errors; // Enabled by default.
        // If `check_errors` is enabled, this function is called if a error occurs.
        using error_hanler_func_t = std::function<void()>;
        extern error_hanler_func_t error_handler_func; // Calls `std::terminate` by default.

        // Calls `glGetError` directly, without relying on macros.
        GLenum call_glGetError();
        // Uses `call_glGetError` to check for errors.
        // If a error occus, calls `error_handler_func`.
        void check_errors_now();

        // Converts various things to strings for logging purposes.
        template <typename T>
        std::string to_string(const T &object)
        {
            char buffer[64];
            buffer[0] = '\0';

            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (std::is_signed_v<T>)
                    std::snprintf(buffer, sizeof buffer, "%lld (0x%llx)", (long long)object, (unsigned long long/*sic*/)object);
                else
                    std::snprintf(buffer, sizeof buffer, "%llu (0x%llx)", (unsigned long long)object, (unsigned long long)object);
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                std::snprintf(buffer, sizeof buffer, "%.17g", (double)object);
            }
            else if constexpr (std::is_pointer_v<T>)
            {
                if constexpr (std::is_same_v<T, const char *>)
                {
                    if (log_string_params)
                    {
                        const char *ptr = object;
                        if (!ptr)
                            return "null";
                        std::string ret;
                        ret += '`';
                        while (*ptr)
                        {
                            if ((unsigned char)*ptr < ' ' || *ptr == 0x7f)
                            {
                                std::snprintf(buffer, sizeof buffer, "\\x%02x", (unsigned char)*ptr);
                                ret += buffer;
                            }
                            else
                            {
                                ret += *ptr;
                            }
                            ptr++;
                        }
                        ret += '`';
                        return ret;
                    }
                }

                std::snprintf(buffer, sizeof buffer, "%p (%llu)", (void *)object, (unsigned long long)object);
            }
            else
            {
                return "??";
            }

            return buffer;
        }
    }
}
