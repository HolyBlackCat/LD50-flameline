#pragma once

#include <csignal>
#include <exception>
#include <type_traits>

#include "errors.h"

namespace Program
{
    class Parachute
    {
        static constexpr int signal_enums[] {SIGSEGV, SIGABRT, SIGINT, SIGTERM, SIGFPE, SIGILL};
        static constexpr int signal_count = std::extent_v<decltype(signal_enums)>;

        inline static bool instance_exists = 0;

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
                    HardError("Exception: ", e.what());
                }
                catch (...)
                {
                    HardError("Unknown exception.");
                }
            }

            HardError("Terminated.");
        }

      public:
        Parachute() // All constructor calls after the first one are no-ops.
        {
            if (instance_exists)
                return;
            instance_exists = 1;

            for (int i = 0; i < signal_count; i++)
                std::signal(signal_enums[i], SignalHandler);

            std::set_terminate(TerminateHandler);
        }

        Parachute(const Parachute &) = delete;
        Parachute &operator=(const Parachute &) = delete;

        static void ForgetExistingInstances() // A single next constructor call will set the handlers again.
        {
            instance_exists = 0;
        }
    };

    static Program::Parachute error_parachute; // Note that this has to be static to prevent static init order fiasco.
}
