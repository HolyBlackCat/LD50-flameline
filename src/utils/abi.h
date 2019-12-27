#pragma once

#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <typeinfo>
#include <utility>

#include <cxxabi.h>

#include "program/errors.h"

namespace Abi
{
    class Demangle
    {
        #ifndef _MSC_VER
        char *buf_ptr = nullptr;
        std::size_t buf_size = 0;
        #endif

      public:
        Demangle() {}

        Demangle(Demangle &&o) noexcept
        #ifndef _MSC_VER
            : buf_ptr(std::exchange(o.buf_ptr, {})), buf_size(std::exchange(o.buf_size, {}))
        #endif
        {}

        Demangle &operator=(Demangle o) noexcept
        {
            #ifndef _MSC_VER
            std::swap(buf_ptr, o.buf_ptr);
            std::swap(buf_size, o.buf_size);
            #else
            (void)o;
            #endif
            return *this;
        }

        ~Demangle()
        {
            #ifndef _MSC_VER
            // Freeing a nullptr is a no-op.
            std::free(buf_ptr);
            #endif
        }

        // Demangles a name.
        // On GCC ang Clang invokes __cxa_demangle, on MSVC returns the string unchanged.
        // The returned pointer remains as long as both the passed string and the class instance are alive.
        [[nodiscard]] const char *operator()(const char *name)
        {
            #ifndef _MSC_VER
            int status = -4;
            const char *ret = abi::__cxa_demangle(name, buf_ptr, &buf_size, &status);
            DebugAssert("Unable to demangle a name.", status != -2);
            if (status != 0) // -1 = out of memory, -2 = invalid string, -3 = invalid usage
                return name;
            return ret;
            #else
            return name;
            #endif
        }
    };

    // Returns the pretty name of a type.
    // For each specific T, caches the name on the first call.
    template <typename T>
    [[nodiscard]] const char *TypeName()
    {
        static Demangle d;
        static const char *ret = d(typeid(T).name());
        return ret;
    }
    template <typename T>
    [[nodiscard]] const char *TypeName(const T &)
    {
        return TypeName<T>();
    }
}
