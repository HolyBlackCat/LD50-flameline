#ifndef PROGRAM_PARACHUTE_H_INCLUDED
#define PROGRAM_PARACHUTE_H_INCLUDED

#include <csignal>
#include <exception>
#include <type_traits>

#include "errors.h"

namespace Program
{
    class Parachute
    {
        Parachute(const Parachute &) = delete;
        Parachute &operator=(const Parachute &) = delete;

        static constexpr int signal_enums[] {SIGSEGV, SIGABRT, SIGINT, SIGTERM, SIGFPE, SIGILL};
        static constexpr int signal_count = std::extent_v<decltype(signal_enums)>;

        using func_t = void(int);
        func_t *old_signal_handlers[signal_count];
        std::terminate_handler old_terminate_handler;

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
        Parachute()
        {
            if (instance_exists)
                Program::HardError("Too many parachutes.");
            instance_exists = 1;

            for (int i = 0; i < signal_count; i++)
                old_signal_handlers[i] = std::signal(signal_enums[i], SignalHandler);

            old_terminate_handler = std::set_terminate(TerminateHandler);
        }

        ~Parachute()
        {
            for (int i = 0; i < signal_count; i++)
            {
                if (old_signal_handlers[i] == SIG_ERR)
                    continue;
                func_t *cur_handler = std::signal(signal_enums[i], old_signal_handlers[i]);
                if (cur_handler != SignalHandler)
                    std::signal(signal_enums[i], cur_handler);
            }

            if (std::get_terminate() == TerminateHandler)
                std::set_terminate(old_terminate_handler);

            instance_exists = 0;
        }
    };
}

#endif
