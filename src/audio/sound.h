#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "macros/maybe_const.h"
#include "program/errors.h"
#include "stream/input.h"

namespace Audio
{
    enum Format
    {
        wav,
        ogg,
    };

    enum Channels
    {
        mono = 1,
        stereo = 2,
    };

    enum BitResolution
    {
        bits_8 = 8, // uint8_t
        bits_16 = 16, // int16_t
    };

    [[nodiscard]] inline int GetBytesPerSample(BitResolution resolution)
    {
        switch (resolution)
        {
            case bits_8: return 1;
            case bits_16: return 2;
        }
        return 1;
    }
    [[nodiscard]] inline int GetBytesPerBlock(BitResolution resolution, Channels channel_count)
    {
        return GetBytesPerSample(resolution) * int(channel_count);
    }

    class Sound
    {
        std::vector<std::uint8_t> data;

        int sampling_rate = 0; // 0 indicates a null sound.
        Channels channel_count = mono;
        BitResolution resolution = bits_8;

      public:
        // Construct an empty sound.
        Sound() {}

        // Construct a sound with a format specified at runtime.
        // Note that the length of the data is measured in blocks. Each block consists of `channel_count` samples, each sample having `resolution` bits in it.
        Sound(int sampling_rate, Channels channel_count, BitResolution resolution, std::size_t block_count, const std::uint8_t *source = nullptr)
            : sampling_rate(sampling_rate), channel_count(channel_count), resolution(resolution)
        {
            std::size_t byte_size = BytesPerBlock() * block_count;
            if (source)
                data.assign(source, source + byte_size);
            else
                data.resize(byte_size);
        }

        // Construct a 8-bit sound.
        // Note that the length of the data is measured in blocks. Each block consists of `channel_count` samples, each sample having 8 bits in it.
        Sound(int sampling_rate, Channels channel_count, std::size_t block_count, const std::uint8_t *source = nullptr)
            : Sound(sampling_rate, channel_count, bits_8, block_count, source)
        {}
        // Construct a 16-bit sound.
        // Note that the length of the data is measured in blocks. Each block consists of `channel_count` samples, each sample having 16 bits in it.
        Sound(int sampling_rate, Channels channel_count, std::size_t block_count, const std::int16_t *source = nullptr)
            : Sound(sampling_rate, channel_count, bits_16, block_count, reinterpret_cast<const std::uint8_t *>(source))
        {}

        // Loads a sound from a stream, according to the specified `format.
        // If `expected_channel_count` is not null, will throw if the received data doesn't have the specified amount of channels.
        // `preferred_resolution` specifies the desired resolution; its effect depends on the format. (Currently it's ignored for WAV, and is used unconditionally for OGG.)
        Sound(Format format, std::optional<Channels> expected_channel_count, Stream::Input input, BitResolution preferred_resolution = bits_16);

        // Returns true if the object holds any data.
        [[nodiscard]] explicit operator bool() const
        {
            return !data.empty();
        }

        // Getters/setters for the state:

        // Get sampling rate.
        [[nodiscard]] int SamplingRate() const {return sampling_rate;}
        // Set sampling rate.
        // This is mutable (unlike other parameters) because it doesn't affect data storage.
        void SetSamplingRate(int new_sampling_rate) {sampling_rate = new_sampling_rate;}

        // Get channel count.
        // No setter is provided, reassign the class to change the value.
        [[nodiscard]] Channels ChannelCount() const {return channel_count;}

        // Get resolution.
        // No setter is provided, reassign the class to change the value.
        [[nodiscard]] BitResolution Resolution() const {return resolution;}

        MAYBE_CONST
        (
            // Get an untyped pointer to the sound data.
            [[nodiscard]] CV std::uint8_t *RawUntypedData() CV {return data.data();}

            // Get a pointer to the sound data, casted to a specific type.
            // Triggers an assertion if the type doesn't match the `Resolution()`.
            template <typename T> requires std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::int16_t>
            [[nodiscard]] CV T *Data() CV
            {
                ASSERT(sizeof(T) == BytesPerSample(), "Type mismatch.");
                return reinterpret_cast<CV T *>(RawUntypedData());
            }
        )


        // Various information that can be computed based on the state:

        // The amount of bytes per sample per channel, either 1 or 2.
        [[nodiscard]] int BytesPerSample() const
        {
            return GetBytesPerSample(resolution);
        }
        // The amount of bytes per block. Equal to the amount of bytes per sample, times channel count.
        [[nodiscard]] int BytesPerBlock() const
        {
            return GetBytesPerBlock(resolution, channel_count);
        }

        // The data size in bytes.
        [[nodiscard]] std::size_t ByteSize() const
        {
            return data.size();
        }

        // The amount of samples. (Corresponding samples from different channels count as one).
        [[nodiscard]] std::size_t BlockCount() const
        {
            return ByteSize() / BytesPerBlock();
        }
    };
}
