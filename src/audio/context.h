#pragma once

#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include "audio/openal.h"
#include "macros/finally.h"
#include "program/errors.h"

namespace Audio
{
    struct ContextAttr
    {
        // Don't modify memory layout, this struct is passed directly to AL.

        // NOTE: AL uses `key == 0` as an indicator of the end of the attribute list (we add it automatically).
        // Make sure you don't have it in your list, or the options that come after it won't be read.
        ALCint key = 0;
        ALCint value = 0;

        // HRTF stands for "head-related transfer function".
        // Improves the 3D sound in your headphones.
        // From looking at AL code, it seems it should be enabled automatically when you're using headphones,
        // but this flag lets you force enable or disable it. Pass null to restore the setting to the default value,
        // which is "don't care".
        // If you're going to add it to the game options, consider a tristate switch (on/off/auto).
        // [[nodiscard]] static ContextAttr Hrtf(std::optional<bool> enable)
        // {
        //     return {ALC_HRTF_SOFT, enable.value_or(ALC_DONT_CARE_SOFT)};
        // }
    };

    class Context
    {
        inline static Context *instance = nullptr;

        struct Data
        {
            ALCdevice *device = nullptr;
            ALCcontext *context = nullptr;
        };
        Data data;

      public:
        Context() {}

        using attribute_list_t = std::vector<ContextAttr>;

        // Creates a context with the default configuration.
        Context(decltype(nullptr)) : Context(attribute_list_t{}) {}

        // Creates a context with the specified configuration.
        // Note that we pass the vector by value, because we need to append a null element at the end before passing it to AL.
        Context(attribute_list_t attributes)
        {
            constexpr int required_major = 1, required_minor = 1;

            // Stop if a context already exists.
            if (instance)
                Program::Error("Attempt to create multiple OpenAL contexts.");

            // Open device.
            data.device = alcOpenDevice(nullptr);
            if (!data.device)
                Program::Error("Unable to create an OpenAL device.");
            FINALLY_ON_THROW( alcCloseDevice(data.device); )

            // Get dynamic library version, stop if it's too old.
            ALCint lib_major, lib_minor;
            alcGetIntegerv(data.device, ALC_MAJOR_VERSION, 1, &lib_major);
            alcGetIntegerv(data.device, ALC_MINOR_VERSION, 1, &lib_minor);
            if (std::tie(lib_major, lib_minor) < std::tie(required_major, required_minor))
                Program::Error(FMT("The OpenAL dynamic library is too old. Expected at least version {}.{}.", required_major, required_minor));

            // Append a terminating zero to the list of attributes.
            if (!attributes.empty())
                attributes.emplace_back().key = 0;

            // Create context.
            data.context = alcCreateContext(data.device, attributes.empty() ? nullptr : &attributes.front().key);
            if (!data.context)
                Program::Error("Unable to create an OpenAL context.\nMake sure your speakers or headphones are connected.");
            FINALLY_ON_THROW( alcDestroyContext(data.context); )

            // Activate the context.
            if (!alcMakeContextCurrent(data.context))
                Program::Error("Unable to activate the OpenAL context.");

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

                instance = nullptr;
            }
        }

        [[nodiscard]] explicit operator bool() const
        {
            return bool(data.device);
        }

        [[nodiscard]] static bool Exists()
        {
            return bool(instance);
        }
        [[nodiscard]] static Context &Get() // This will throw if the context doesn't exist.
        {
            if (!instance)
                Program::Error("Attempt to use an OpenAL context before it was created.");
            return *instance;
        }

        [[nodiscard]] ALCdevice *DeviceHandle() const
        {
            return data.device;
        }
        [[nodiscard]] ALCcontext *ContextHandle() const
        {
            return data.context;
        }

        // Changes configuration for an existing context.
        // Note that we pass the vector by value, because we need to append a null element at the end before passing it to AL.
        // void Reconfigure(attribute_list_t attributes)
        // {
        //     if (!attributes.empty())
        //         attributes.emplace_back().key = 0;

        //     if (alcResetDeviceSOFT(data.device, attributes.empty() ? nullptr : &attributes.front().key))
        //         Program::Error("Unable to change an audio device configuration.");
        // }
    };
}
