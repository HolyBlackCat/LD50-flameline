#ifndef EXIT_H_INCLUDED
#define EXIT_H_INCLUDED

#include <cstdlib>

namespace Program
{
    [[noreturn]] inline void Exit(int code = 0)
    {
        std::exit(code);
    }
}

#endif
