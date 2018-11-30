#pragma once

#include <tuple>
#include <utility>

#include <AL/al.h>
#include <AL/alc.h>

#include "program/errors.h"
#include "utils/finally.h"

namespace Audio
{
    class Context
    {
        inline static Context *instance = 0;

        struct Data
        {
            ALCdevice *device = 0;
            ALCcontext *context = 0;
        };
        Data data;

      public:
        Context(decltype(nullptr)) {}

        Context()
        {
            constexpr int required_major = 1, required_minor = 1;

            // Open device.
            data.device = alcOpenDevice(0);
            if (!data.device)
                Program::Error("Unable to create an OpenAL device.");
            FINALLY_ON_THROW( alcCloseDevice(data.device); )

            // Get dynamic library version, stop if it's too old.
            ALCint lib_major, lib_minor;
            alcGetIntegerv(data.device, ALC_MAJOR_VERSION, 1, &lib_major);
            alcGetIntegerv(data.device, ALC_MINOR_VERSION, 1, &lib_minor);
            if (std::tie(lib_major, lib_minor) < std::tie(required_major, required_minor))
                Program::Error("OpenAL dynamic library is too old. Expected at least version ", required_major, ".", required_minor, ".");

            // Create context.
            data.context = alcCreateContext(data.device, 0);
            if (!data.context)
                Program::Error("Unable to create an OpenAL context.");
            FINALLY_ON_THROW( alcDestroyContext(data.context); )

            // Activate the context.
            if (!alcMakeContextCurrent(data.context))
                Program::Error("Unable to activate an OpenAL context.");

            // Save the instance pointer.
            instance = this;
        }

        Context(Context &&other) noexcept : data(std::exchange(other.data, {}))
        {
            if (instance == &other)
                instance = this;
        }
        Context &operator=(Context other) noexcept
        {
            std::swap(data, other.data);
            if (instance == &other)
                instance = this;
            return *this;
        }

        ~Context()
        {
            if (data.context)
            {
                alcDestroyContext(data.context);
                alcCloseDevice(data.device);

                instance = 0;
            }
        }

        explicit operator bool() const
        {
            return bool(data.device);
        }

        static Context &Get() // This will throw if the context doesn't exist.
        {
            if (!instance)
                Program::Error("Attempt to use an OpenAL context before it was created.");
            return *instance;
        }

        ALCdevice *DeviceHandle() const
        {
            return data.device;
        }
        ALCcontext *ContextHandle() const
        {
            return data.context;
        }
    };
}
