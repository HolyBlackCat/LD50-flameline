// CGLFL (configurable OpenGL function loader)
//   by Egor Mikhailov <blckcat@inbox.ru>
// License: zlib

#include <algorithm>
#include <array>
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>

#include <cglfl/cglfl.hpp>
#include <cglfl_generated/macros_internal.hpp>

namespace cglfl
{
    static bool load_func(int index, std::vector<int> &rollback_list, function_loader_t &loader, const char *name)
    {
        auto &ptr = context_pointer->functions[index];
        if (ptr)
            return true;

        ptr = loader(name);
        if (!ptr)
            return false;

        rollback_list.push_back(index);
        return true;
    }

    #undef CGLFL_CALL
    #define CGLFL_CALL(i, func, ret, n, param_names, params) && load_func(i, rollback_list, loader, #func)
    bool load(function_loader_t loader, bool throw_on_failure)
    {
        std::vector<int> rollback_list;
        rollback_list.reserve(CGLFL_PRIMARY_FUNC_COUNT);

        bool ok = true CGLFL_PRIMARY_FUNCS;
        if (!ok)
        {
            for (int index : rollback_list)
                context_pointer->functions[index] = nullptr;
            if (throw_on_failure)
                throw std::runtime_error("cglfl: Unable to load primary OpenGL functions.");
            return false;
        }

        return true;
    }

    bool load_extension(function_loader_t loader, std::string extension, bool throw_on_failure)
    {
        using func_pair_t = std::pair<std::string, bool (*)(function_loader_t &, bool)>;
        static std::array<func_pair_t, CGLFL_EXT_COUNT> ext_funcs
        {
            #define CGLFL_X(ext) func_pair_t{#ext, [](function_loader_t &loader, bool throw_on_failure) -> bool  \
            {                                                                                                    \
                std::vector<int> rollback_list;                                                                  \
                rollback_list.reserve(CGLFL_IMPL_CAT(CGLFL_EXT_FUNC_COUNT_, ext));                               \
                bool ok = true CGLFL_IMPL_CAT(CGLFL_EXT_FUNCS_, ext);                                            \
                if (!ok)                                                                                         \
                {                                                                                                \
                    for (int index : rollback_list)                                                              \
                        context_pointer->functions[index] = nullptr;                                             \
                    if (throw_on_failure)                                                                        \
                        throw std::runtime_error("cglfl: Unable to load OpenGL functions for extension: " #ext); \
                    return false;                                                                                \
                }                                                                                                \
                return true;                                                                                     \
            }},
            CGLFL_EXTS(CGLFL_X)
            #undef CGLFL_X
        };

        auto it = std::lower_bound(ext_funcs.begin(), ext_funcs.end(), extension, [](const func_pair_t &a, const std::string &b)
        {
            return a.first < b;
        });

        if (it == ext_funcs.end() || it->first != extension)
        {
            if (throw_on_failure)
                throw std::runtime_error("cglfl: Unknown extension: " + extension);
            return false;
        }

        return it->second(loader, throw_on_failure);
    }

    bool load_all(function_loader_t loader, bool throw_on_failure)
    {
        #define CGLFL_X(ext) && load_extension(loader, #ext, throw_on_failure)
        return load(loader, throw_on_failure) CGLFL_EXTS(CGLFL_X);
        #undef CGLFL_X
    }

    #ifndef CGLFL_NO_DEFAULT_CONTEXT
    static context default_context_object;
    context *default_context() {return &default_context_object;}
    context *context_pointer = &default_context_object;
    #else
    context *default_context() {return nullptr;}
    context *context_pointer = nullptr;
    #endif

    namespace debug
    {
        log_func_t log_func = [](const char *str)
        {
            std::printf("%s\n", str);
        };

        void log(const char *string)
        {
            if (log_func)
                log_func(string);
        }
        void log(const std::string &string)
        {
            log(string.c_str());
        }

        bool log_calls = true;
        bool log_string_params = true;

        bool check_errors = true;
        error_hanler_func_t error_handler_func = []
        {
            std::terminate();
        };

        void check_errors_now()
        {
            if (GLenum err = call_glGetError())
            {
                std::string message;
                do
                {
                    if (message.size())
                        message += ", ";

                    switch (err)
                    {
                        case GL_INVALID_ENUM:                  message += "invalid enum";                  break;
                        case GL_INVALID_VALUE:                 message += "invalid value";                 break;
                        case GL_INVALID_OPERATION:             message += "invalid operation";             break;
                        case GL_OUT_OF_MEMORY:                 message += "out of memory";                 break;
                        #ifdef GL_INVALID_FRAMEBUFFER_OPERATION
                        case GL_INVALID_FRAMEBUFFER_OPERATION: message += "invalid framebuffer operation"; break;
                        #endif
                        #ifdef GL_STACK_UNDERFLOW
                        case GL_STACK_UNDERFLOW:               message += "stack underflow";               break;
                        #endif
                        #ifdef GL_STACK_OVERFLOW
                        case GL_STACK_OVERFLOW:                message += "stack overflow";                break;
                        #endif
                        #ifdef GL_TABLE_TOO_LARGE
                        case GL_TABLE_TOO_LARGE:               message += "table too large";               break;
                        #endif
                        default:                               message += "???";                           break;
                    }
                }
                while ((err = call_glGetError()));

                log("");
                log(std::string(50, '#'));
                log("OPENGL ERROR: " + message);
                log(std::string(50, '#'));

                if (error_handler_func)
                    error_handler_func();
            }
        }

        GLenum call_glGetError()
        {
            #undef CGLFL_CALL
            #define CGLFL_CALL CGLFL_CALL_DEFAULT
            return glGetError();
        }
    }
}
