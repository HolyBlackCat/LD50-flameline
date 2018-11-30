#pragma once

#include <cstring>
#include <vector>

#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>

#include "program/errors.h"
#include "utils/byte_order.h"
#include "utils/finally.h"
#include "utils/memory_access.h"
#include "utils/memory_file.h"

namespace Audio
{
    enum Format {wav, ogg};
    enum Channels {mono = 1, stereo = 2};
    enum Bits {bits_8 = 8, bits_16 = 16};

    class Sound
    {
        std::vector<std::uint8_t> data;

        Channels channel_count = mono;
        Bits bits_per_sample = bits_8;
        int sampling_rate = 8000;

      public:
        Sound()
        {
            (void)OV_CALLBACKS_DEFAULT;
            (void)OV_CALLBACKS_NOCLOSE;
            (void)OV_CALLBACKS_STREAMONLY;
            (void)OV_CALLBACKS_STREAMONLY_NOCLOSE;
        }
        Sound(std::size_t samples, Channels channel_count, Bits bits_per_sample, int sampling_rate, std::uint8_t *source = 0)
            : channel_count(channel_count), bits_per_sample(bits_per_sample), sampling_rate(sampling_rate)
        {
            std::size_t byte_count = samples * BitsPerSample();
            if (source)
                data = std::vector<uint8_t>(source, source + byte_count);
            else
                data = std::vector<uint8_t>(byte_count);
        }
        Sound(Format format, MemoryFile file, Bits preferred_bits_per_sample = bits_16); // `preferred_bits_per_sample` is ignored for WAV files.

        Channels ChannelCount() const
        {
            return channel_count;
        }
        Bits BitsPerSample() const
        {
            return bits_per_sample;
        }
        int SamplingRate() const
        {
            return sampling_rate;
        }
        int ByteCount() const
        {
            return data.size();
        }
        int BytesPerSample() const
        {
            int ret = int(channel_count);
            if (bits_per_sample == bits_16)
                ret *= 2;
            return ret;
        }
        int SampleCount() const
        {
            return ByteCount() / BytesPerSample();
        }

        void SetSamplingRate(int new_sampling_rate)
        {
            sampling_rate = new_sampling_rate;
        }

        const uint8_t *Data() const
        {
            return data.data();
        }
    };
}
