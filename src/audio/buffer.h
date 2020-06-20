#pragma once

#include <cstdint>
#include <utility>

#include "audio/openal.h"
#include "audio/sound.h"
#include "macros/finally.h"
#include "program/errors.h"

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
        Buffer() {}

        Buffer(decltype(nullptr))
        {
            alGenBuffers(1, &data.handle);
            if (!data.handle)
                Program::Error("Unable to create an audio buffer.");
            // Not needed because there is no code below this point:
            // FINALLY_ON_THROW( alDeleteBuffers(1, &value); )
        }
        Buffer(const Sound &sound) : Buffer(nullptr)
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

        [[nodiscard]] explicit operator bool() const
        {
            return bool(data.handle);
        }

        [[nodiscard]] ALuint Handle() const
        {
            return data.handle;
        }

        // Sets the data from memory, with the format specified at runtime.
        // Note that the length of the data is measured in blocks. Each block consists of `channel_count` samples, each sample having `resolution` bits in it.
        void SetData(int sampling_rate, Channels channel_count, BitResolution resolution, std::size_t block_count, const std::uint8_t *source = nullptr)
        {
            ASSERT(*this, "Attempt to use a null audio buffer.");
            if (!*this)
                return;

            ALenum format;
            if (channel_count == mono)
            {
                if (resolution == bits_8)
                    format = AL_FORMAT_MONO8;
                else // bits_16
                    format = AL_FORMAT_MONO16;
            }
            else // stereo
            {
                if (resolution == bits_8)
                    format = AL_FORMAT_STEREO8;
                else // bits_16
                    format = AL_FORMAT_STEREO16;
            }

            alBufferData(data.handle, format, source, GetBytesPerBlock(resolution, channel_count) * block_count, sampling_rate);
        }
        // Sets the data from memory, in the 8-bit format.
        // Note that the length of the data is measured in blocks. Each block consists of `channel_count` samples, each sample having 8 bits in it.
        void SetData(int sampling_rate, Channels channel_count, std::size_t block_count, const std::uint8_t *source = nullptr)
        {
            SetData(sampling_rate, channel_count, bits_8, block_count, source);
        }
        // Sets the data from memory, in the 16-bit format.
        // Note that the length of the data is measured in blocks. Each block consists of `channel_count` samples, each sample having 16 bits in it.
        void SetData(int sampling_rate, Channels channel_count, std::size_t block_count, const std::int16_t *source = nullptr)
        {
            SetData(sampling_rate, channel_count, bits_16, block_count, reinterpret_cast<const std::uint8_t *>(source));
        }

        // Copies the data from the `sound` into the buffer.
        void SetData(const Sound &sound)
        {
            SetData(sound.SamplingRate(), sound.ChannelCount(), sound.Resolution(), sound.BlockCount(), sound.RawUntypedData());
        }
    };
}
