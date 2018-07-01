#ifndef STRINGS_H_INCLUDED
#define STRINGS_H_INCLUDED

#include <string>
#include <sstream>

namespace Strings
{
    class impl
    {
        static std::ostringstream &stream()
        {
            static std::ostringstream ret;
            return ret;
        }
        static std::stringstream::fmtflags default_flags()
        {
            static std::stringstream::fmtflags ret = stream().flags();
            return ret;
        }

        template <typename ...P> friend std::string Str(const P &...);
        template <typename ...P> friend std::string Str_(const P &...);
    };

    // Str() resets stringstream flags before doing anything.
    // Str_() does not.

    template <typename ...P> [[nodiscard]] std::string Str(const P &... params)
    {
        impl::stream().clear();
        impl::stream().str("");
        impl::stream().flags(impl::default_flags());
        impl::stream().width(0);
        (impl::stream() << ... << params);
        return impl::stream().str();
    }
    template <typename ...P> [[nodiscard]] std::string Str_(const P &... params)
    {
        impl::stream().clear();
        impl::stream().str("");
        (impl::stream() << ... << params);
        return impl::stream().str();
    }
}

using Strings::Str;
using Strings::Str_;

#endif
