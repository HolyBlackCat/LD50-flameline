#include "errors.h"

#include <csignal>
#include <type_traits>

namespace Program
{
    static void SignalHandler(int sig)
    {
        switch (sig)
        {
          case SIGABRT:
          case SIGINT:
          case SIGTERM:
            Exit(0);
            break;
          case SIGFPE:
            HardError("Signal: Floating point exception.");
            break;
          case SIGILL:
            HardError("Signal: Illegal instruction.");
            break;
          case SIGSEGV:
            HardError("Signal: Segmentation fault.");
            break;
          default:
            HardError("Signal: Unknown.");
            break;
        }
    }

    static void TerminateHandler()
    {
        if (std::exception_ptr e = std::current_exception())
        {
            try
            {
                std::rethrow_exception(e);
            }
            catch (std::exception &e)
            {
                HardError("Exception:\n", e.what());
            }
            catch (...)
            {
                HardError("Unknown exception.");
            }
        }

        HardError("Terminated.");
    }

    static bool handers_set = 0;

    void SetErrorHandlers(bool only_if_not_set_before)
    {
        if (only_if_not_set_before && handers_set)
            return;

        static constexpr int signal_enums[] {SIGSEGV, SIGABRT, SIGINT, SIGTERM, SIGFPE, SIGILL};

        for (int sig : signal_enums)
            std::signal(sig, SignalHandler);

        std::set_terminate(TerminateHandler);

        handers_set = 1;
    }
}
