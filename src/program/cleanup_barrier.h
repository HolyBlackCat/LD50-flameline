#ifndef CLEANUP_BARRIER_H_INCLUDED
#define CLEANUP_BARRIER_H_INCLUDED

#include "exit.h"

namespace Program
{
    class CleanupBarrier // Stops the program when destroyed.
    {
      public:
        ~CleanupBarrier()
        {
            ImmediateExit();
        };
    };
}

#endif
