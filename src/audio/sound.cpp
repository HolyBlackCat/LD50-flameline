#include "sound.h"

#include <string_view>

#include <vorbis/vorbisfile.h>

#include "macros/finally.h"
#include "strings/format.h"
#include "utils/robust_math.h"

namespace Audio
{
    Sound::Sound(Format format, std::optional<Channels> expected_channel_count, Stream::Input input, BitResolution preferred_resolution)
    {
        auto CheckChannelCount = [&]
        {
            if (expected_channel_count && *expected_channel_count != channel_count)
            {
                Program::Error(FMT("{}Expected a {} sound, but got {}.", input.GetExceptionPrefix(),
                    (*expected_channel_count == mono ? "mono" : "stereo"), (channel_count == mono ? "mono" : "stereo")));
            }
        };

        switch (format)
        {
          case wav:
            try
            {
                input.WantExceptionPrefixStyle(Stream::with_location);
                input.WantLocationStyle(Stream::byte_offset);

                input.DiscardChars("RIFF");

                if (auto val = input.ReadLittle<std::uint32_t>(); val + val % 2 != input.RemainingBytes())
                    Program::Error(input.GetExceptionPrefix() + "Incorrect size specified in the RIFF header.");

                input.DiscardChars("WAVE");

                bool got_format_chunk = false;
                bool got_data_chunk = false; // If this is true, then `got_format_chunk` must be true.

                // Loop over all chunks in the file.
                while (input.RemainingBytes())
                {
                    char chunk_name_buf[4];
                    input.Read(chunk_name_buf, sizeof chunk_name_buf);
                    std::string_view chunk_name(chunk_name_buf, sizeof chunk_name_buf);

                    auto chunk_size = input.ReadLittle<std::uint32_t>();
                    if (chunk_size > input.RemainingBytes())
                        Program::Error(input.GetExceptionPrefix() + "Chunk size is too large.");

                    if (chunk_name == "fmt ") // Note the space.
                    {
                        if (got_format_chunk)
                            Program::Error(input.GetExceptionPrefix() + "A repeated format chunk.");
                        // `got_data_chunk` will be false here, no need to check.
                        got_format_chunk = true;

                        if (input.ReadLittle<std::uint16_t>() != 1)
                            Program::Error(input.GetExceptionPrefix() + "Compressed files are not supported.");

                        // We check chunk size after checking the data format, to give user a prettier error message.
                        if (chunk_size != 16 && chunk_size != 18)
                            Program::Error(input.GetExceptionPrefix() + "Invalid format chunk size.");

                        auto channels = input.ReadLittle<std::uint16_t>();
                        if (channels != 1 && channels != 2)
                            Program::Error(input.GetExceptionPrefix() + "Only mono and stereo sound is supported.");
                        channel_count = Channels(channels);
                        CheckChannelCount();

                        sampling_rate = input.ReadLittle<std::uint32_t>();
                        if (sampling_rate <= 0)
                            Program::Error(input.GetExceptionPrefix() + "Invalid sample rate.");

                        input.Skip(4); // Bytes per second (possibly average). We don't care about that.
                        input.Skip(2); // Bytes per sample for all channels combined. We don't need this value.

                        auto bits_per_sample_per_channel = input.ReadLittle<std::uint16_t>();
                        if (bits_per_sample_per_channel != 8 && bits_per_sample_per_channel != 16)
                            Program::Error(input.GetExceptionPrefix() + "Unsupported resolution, expected 8 or 16 bits per sample.");
                        resolution = BitResolution(bits_per_sample_per_channel);

                        if (chunk_size == 18)
                            input.DiscardBytes({0,0});
                    }
                    else if (chunk_name == "data")
                    {
                        if (!got_format_chunk)
                            Program::Error(input.GetExceptionPrefix() + "No format chunk found before a data chunk.");
                        if (got_data_chunk)
                            Program::Error(input.GetExceptionPrefix() + "A repeated data chunk.");
                        got_data_chunk = true;

                        if (chunk_size % BytesPerBlock() != 0)
                            Program::Error(input.GetExceptionPrefix() + "Data size is not a multiple of block size.");

                        data.resize(chunk_size);
                        switch (resolution)
                        {
                          case bits_8:
                            input.Read(Data<std::uint8_t>(), chunk_size);
                            break;
                          case bits_16:
                            input.ReadLittle(Data<std::int16_t>(), chunk_size / BytesPerSample());
                        }
                    }
                    else
                    {
                        // Skip any unknown chunks.
                        input.Skip(chunk_size);
                    }

                    // Skip padding.
                    // It normally should be 0, but I'm not going to validate it.
                    // It's a bad idea according to https://github.com/taglib/taglib/issues/882
                    if (chunk_size % 2 == 1)
                        input.SkipOne();
                }

                // Check if got the necessary chunks.
                // We don't need to check `got_format_chunk` because it's implied by `got_data_chunk`.
                if (!got_data_chunk)
                    Program::Error("The data chunk is missing.");
            }
            catch (std::exception &e)
            {
                Program::Error(FMT("While reading a wav sound from `{}`:\n{}", input.GetTarget(), e.what()));
            }
            break;
          case ogg:
            try
            {
                // Stream exceptions aren't supposed to escape the callbacks anyway,
                // might as well make constructing them as cheap as possible.
                input.WantExceptionPrefixStyle(Stream::no_prefix);

                // Construct callbacks.
                ov_callbacks callbacks;
                callbacks.close_func = nullptr;
                callbacks.tell_func = [](void *stream_ptr) -> long
                {
                    try
                    {
                        long ret;
                        if (Robust::conversion_fails(static_cast<Stream::Input *>(stream_ptr)->Position(), ret))
                            return -1;
                        return ret;
                    }
                    catch (...)
                    {
                        return -1;
                    }
                };
                callbacks.seek_func = [](void *stream_ptr, std::int64_t offset, int mode) -> int
                {
                    try
                    {
                        std::ptrdiff_t converted_offset;
                        if (Robust::conversion_fails(offset, converted_offset))
                            return -1;

                        Stream::SeekMode converted_mode;
                        switch (mode)
                        {
                          case SEEK_SET:
                            converted_mode = Stream::absolute;
                            break;
                          case SEEK_CUR:
                            converted_mode = Stream::relative;
                            break;
                          case SEEK_END:
                            converted_mode = Stream::end;
                            break;
                          default:
                            return -1;
                        }

                        static_cast<Stream::Input *>(stream_ptr)->Seek(converted_offset, converted_mode);
                        return 0;
                    }
                    catch (...)
                    {
                        return -1;
                    }
                };
                callbacks.read_func = [](void *buffer, std::size_t elem_size, std::size_t elem_count, void *stream_ptr) -> std::size_t
                {
                    try
                    {
                        if (elem_size == 0 || elem_count == 0)
                            return 0;

                        auto &stream = *static_cast<Stream::Input *>(stream_ptr);

                        std::size_t total_size;
                        bool enough_data = true;

                        // If the read size is larger than the remaining amount of bytes
                        // OR if the calculation of `total_size` overflowed, clamp the read size.
                        if ((Robust::value(elem_size) * Robust::value(elem_count) >>= total_size) || total_size > stream.RemainingBytes())
                        {
                            total_size = stream.RemainingBytes();
                            enough_data = false;
                        }

                        stream.Read(static_cast<char *>(buffer), total_size);

                        if (enough_data)
                            return elem_count;
                        else
                            return total_size / elem_size;
                    }
                    catch (...)
                    {
                        return -1;
                    }
                };

                // Open a file with those callbacks.
                OggVorbis_File ogg_file_handle;
                switch (ov_open_callbacks(&input, &ogg_file_handle, nullptr, 0, callbacks))
                {
                  case 0:
                    break;
                  case OV_EREAD:
                    Program::Error("Unable to read data from the stream.");
                    break;
                  case OV_ENOTVORBIS:
                    Program::Error("This is not a vorbis sound.");
                    break;
                  case OV_EVERSION:
                    Program::Error("Vorbis version mismatch.");
                    break;
                  case OV_EBADHEADER:
                    Program::Error("Invalid header.");
                    break;
                  case OV_EFAULT:
                    Program::Error("Internal vorbis error.");
                    break;
                  default:
                    Program::Error("Unknown vorbis error.");
                    break;
                }
                FINALLY( ov_clear(&ogg_file_handle); )


                // Get some info about the file. No cleanup appears to be necessary.
                vorbis_info *info = ov_info(&ogg_file_handle, -1);
                if (!info)
                    Program::Error("Unable to get information about the file.");

                // Get channel count.
                if (info->channels != 1 && info->channels != 2)
                    Program::Error("The file has too many channels. Only mono and stereo are supported.");
                channel_count = Channels(info->channels);
                CheckChannelCount();

                // Get frequency.
                if (Robust::conversion_fails(info->rate, sampling_rate))
                    Program::Error("The sample rate is too high.");

                // Get the block count.
                auto total_block_count = ov_pcm_total(&ogg_file_handle, -1);
                if (total_block_count == OV_EINVAL)
                    Program::Error("Unable to determine the file length.");

                // Copy bit resolution from the parameter.
                resolution = preferred_resolution;

                // Compute the necessary storage size.
                std::size_t storage_size;
                if (Robust::value(total_block_count).cast_to<std::size_t>() * Robust::value(BytesPerBlock()).weakly_typed() >>= storage_size)
                    Program::Error("The file is too long.");

                data.resize(storage_size);

                std::size_t current_offset = 0;

                // An index of the current bitstream (basically a section) of the file.
                // When it changes, the amount of channels and/or sample rate can also change; if it happens, we throw an exception.
                int old_bitstream_index = -1;

                while (current_offset < storage_size)
                {
                    int bitstream_index;
                    auto segment_size = ov_read(&ogg_file_handle, reinterpret_cast<char *>(data.data() + current_offset), storage_size - current_offset, 0/*little endian*/,
                        BytesPerSample(), resolution == bits_16/*true means numbers are signed*/, &bitstream_index);

                    switch (segment_size)
                    {
                      case 0:
                        Program::Error("Unexpected end of file.");
                        break;
                      case OV_HOLE:
                        Program::Error("The file is corrupted.");
                        break;
                      case OV_EBADLINK:
                        Program::Error("Bad link.");
                        break;
                      case OV_EINVAL:
                        Program::Error("Invalid header.");
                        break;
                    }
                    current_offset += segment_size;

                    if (bitstream_index != old_bitstream_index)
                    {
                        old_bitstream_index = bitstream_index;

                        info = ov_info(&ogg_file_handle, -1); // `-1` means the current bitstream, we could also use `bitstream_index` here.
                        if (!info)
                            Program::Error("Unable to get information about a section of the file.");
                        if (Robust::not_equal(info->channels, int(channel_count)))
                            Program::Error("Channel count has changed in the middle of the file.");
                        if (Robust::not_equal(info->rate, sampling_rate))
                            Program::Error("Sampling rate has changed in the middle of the file.");
                    }
                }

            }
            catch (std::exception &e)
            {
                Program::Error(FMT("While reading a vorbis sound from `{}`:\n{}", input.GetTarget(), e.what()));
            }
            break;
        }
    }
}
