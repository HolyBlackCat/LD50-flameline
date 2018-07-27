#ifndef EXIT_H_INCLUDED
#define EXIT_H_INCLUDED

#include <cstdlib>

namespace Program
{
    [[noreturn]] inline void ImmediateExit(int code = 0)
    {
        std::_Exit(code);
    }

    [[noreturn]] inline void Exit(int code = 0)
    {
        static bool first = 1;
        if (first)
        {
            first = 0;
            std::exit(code);
        }
        else // This might be called from some destructor.
        {
            ImmediateExit(code);
        }
    }
}

#endif
