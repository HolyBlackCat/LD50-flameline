#pragma once

#include <cstdint>
#include <utility>

#include <AL/al.h>

#include "audio/sound.h"
#include "program/errors.h"
#include "utils/finally.h"

namespace Audio
{
    class Buffer
    {
        struct Data
        {
            ALuint handle = 0;
        };
        Data data;

      public:
        Buffer(decltype(nullptr)) {}

        Buffer()
        {
            alGenBuffers(1, &data.handle);
            if (!data.handle)
                Program::Error("Unable to create an audio buffer.");
            // Not needed because there is no code below this point:
            // FINALLY_ON_THROW( alDeleteBuffers(1, &value); )
        }
        Buffer(const Sound &sound) : Buffer()
        {
            SetData(sound);
        }

        Buffer(Buffer &&other) noexcept : data(std::exchange(other.data, {})) {}
        Buffer &operator=(Buffer other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        ~Buffer()
        {
            // Not sure if `alDeleteBuffers` is a no-op if you pass 0 to it. Better be safe.
            if (data.handle)
                alDeleteBuffers(1, &data.handle);
        }

        explicit operator bool() const
        {
            return bool(data.handle);
        }

        ALuint Handle() const
        {
            return data.handle;
        }

        void SetData(Bits bits_per_sample, Channels channel_count, int sampling_rate, int byte_count, const std::uint8_t *source = 0)
        {
            if (!*this)
                return;

            ALenum format;
            if (bits_per_sample == bits_8 && channel_count == mono)
                format = AL_FORMAT_MONO8;
            else if (bits_per_sample == bits_16 && channel_count == mono)
                format = AL_FORMAT_MONO16;
            else if (bits_per_sample == bits_8 && channel_count == stereo)
                format = AL_FORMAT_STEREO8;
            else /* bits_per_sample == bits_16 && channel_count == stereo */
                format = AL_FORMAT_STEREO16;

            alBufferData(data.handle, format, source, byte_count, sampling_rate);
        }
        void SetData(const Sound &sound)
        {
            SetData(sound.BitsPerSample(), sound.ChannelCount(), sound.SamplingRate(), sound.ByteCount(), sound.Data());
        }
    };
}
