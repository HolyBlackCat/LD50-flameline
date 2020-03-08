#pragma once

#include <array>
#include <cstddef>

#include "meta/constexpr_hash.h"

namespace Meta
{
    namespace impl
    {
        template <typename T>
        constexpr const auto &RawTypeName()
        {
            #ifdef _MSC_VER
            return __FUNCSIG__;
            #else
            return __PRETTY_FUNCTION__;
            #endif
        }

        struct RawTypeNameFormat
        {
            std::size_t leading_junk = 0, trailing_junk = 0;
        };

        // Returns `false` on failure.
        inline constexpr bool GetRawTypeNameFormat(RawTypeNameFormat *format)
        {
            const auto &str = RawTypeName<int>();
            for (std::size_t i = 0;; i++)
            {
                if (str[i] == 'i' && str[i+1] == 'n' && str[i+2] == 't')
                {
                    if (format)
                    {
                        format->leading_junk = i;
                        format->trailing_junk = sizeof(str)-i-3-1; // `3` is the length of "int", `1` is the space for the null terminator.
                    }
                    return true;
                }
            }
            return false;
        }

        inline static constexpr RawTypeNameFormat format =
        []{
            static_assert(GetRawTypeNameFormat(nullptr), "Unable to figure out how to generate type names on this compiler.");
            RawTypeNameFormat format;
            GetRawTypeNameFormat(&format);
            return format;
        }();
    }

    // Returns the type name in a `std::array<char, N>` (null-terminated).
    // Don't use this unless you need the value to be constexpr, since if used in
    // a non-constexpr context in a non-optimized build, it might add runtime overhead.
    template <typename T>
    [[nodiscard]] constexpr auto CexprTypeName()
    {
        constexpr std::size_t len = sizeof(impl::RawTypeName<T>()) - impl::format.leading_junk - impl::format.trailing_junk;
        std::array<char, len> name{};
        for (std::size_t i = 0; i < len-1; i++)
            name[i] = impl::RawTypeName<T>()[i + impl::format.leading_junk];
        return name;
    }

    // Returns the type name.
    template <typename T>
    [[nodiscard]] const char *TypeName()
    {
        static constexpr auto name = CexprTypeName<T>();
        return name.data();
    }
    // Returns the type name. The parameter is used only to deduce `T`.
    template <typename T>
    [[nodiscard]] const char *TypeName(const T &)
    {
        return TypeName<T>();
    }

    /* Alternative `__cxa_demangle`-based implementation.

        #include <cxxabi.h>

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
    */

    // Returns a hash of the type name.
    // `hash_t` is `uint32_t`.
    template <typename T>
    [[nodiscard]] constexpr hash_t TypeHash(hash_t seed = 0)
    {
        constexpr auto name = CexprTypeName<T>();
        return cexpr_hash(name.data(), name.size() - 1/*null-terminator*/, seed);
    }

    // Hash test:
    // static_assert(TypeHash<int>() == 3464699359);
}
