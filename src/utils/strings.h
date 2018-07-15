#ifndef STRINGS_H_INCLUDED
#define STRINGS_H_INCLUDED

#include <string>
#include <sstream>

namespace Strings
{
    template <typename ...P> [[nodiscard]] std::string Str(const P &... params)
    {
        std::ostringstream stream;
        (stream << ... << params);
        return stream.str();
    }
}

using Strings::Str;

#endif
