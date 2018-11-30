#include "sound.h"

#include <exception>
#include <limits>

#include <vorbis/vorbisfile.h>

namespace Audio
{
    Sound::Sound(Format format, MemoryFile file, Bits preferred_bits_per_sample)
    {
        try
        {
            switch (format)
            {
              case wav:
                {
                    // Stop if the file is too small.
                    if (file.size() < 44) // 44 is the size of WAV header.
                        Program::Error("The file is too small to be valid.");

                    const std::uint8_t *ptr = file.data();

                    // Stop if no `RIFF` label.
                    if (std::memcmp(ptr, "RIFF", 4))
                        Program::Error("No \"RIFF\" label.");
                    ptr += 4;

                    // Read the remaining file length.
                    std::uint32_t expected_file_data_length = Memory::ReadLittle<std::uint32_t>(ptr);
                    ptr += 4;

                    // Stop if the remaining length is too small.
                    if (expected_file_data_length < 36)
                        Program::Error("Invalid file length specified in the header.");
                    expected_file_data_length -= 36;

                    // Stop if no `WAVE` label.
                    if (std::memcmp(ptr, "WAVE", 4))
                        Program::Error("No \"WAVE\" label.");
                    ptr += 4;

                    // Stop if no `fmt ` label.
                    if (std::memcmp(ptr, "fmt ", 4))
                        Program::Error("No \"fmt \" label.");
                    ptr += 4;

                    // Stop if the remining size of `fmt ` section doesn't match the supported format.
                    if (Memory::ReadLittle<std::uint32_t>(ptr) != 16)
                        Program::Error("This format of WAV files is not supported.");
                    ptr += 4;

                    // Stop if the file is compressed or has floating-point samples.
                    if (Memory::ReadLittle<std::uint16_t>(ptr) != 1)
                        Program::Error("File is compresssed or uses floating-point samples. This is not supported.");
                    ptr += 2;

                    // Get channel count, stop if the file is neither mono nor stereo.
                    std::uint16_t file_channels = Memory::ReadLittle<std::uint16_t>(ptr);
                    if (file_channels != 1 && file_channels != 2)
                        Program::Error("The file must be mono or stereo.");
                    ptr += 2;
                    channel_count = Channels(file_channels);

                    // Get sampling rate, stop if it's too large.
                    std::uint32_t file_sampling_rate = Memory::ReadLittle<std::uint32_t>(ptr);
                    if (file_sampling_rate > std::numeric_limits<int>::max())
                        Program::Error("Invalid sampling rate specified.");
                    ptr += 4;
                    sampling_rate = file_sampling_rate;

                    // Skip useless data:
                    // 4 bytes: The amount of bytes per second.
                    // 2 bytes: Bytes per sample for all channels combined.
                    ptr += 6;

                    // Get the amount of bits per single channel sample, stop if it's neither 8 nor 16.
                    std::uint16_t file_bits = Memory::ReadLittle<std::uint16_t>(ptr);
                    if (file_bits != 8 && file_bits != 16)
                        Program::Error("The file must use 8 or 16 bits per sample.");
                    ptr += 2;
                    bits_per_sample = Bits(file_bits);

                    // Stop if no `data` label.
                    if (std::memcmp(ptr, "data", 4))
                        Program::Error("No \"data\" label.");
                    ptr += 4;

                    // Read the remaining file length.
                    std::uint32_t file_data_length = Memory::ReadLittle<std::uint32_t>(ptr);
                    // Stop if it doesn't match the length stored at the beginning of the file.
                    if (file_data_length != expected_file_data_length)
                        Program::Error("The file contains additional unsupported sectons or is corrupted.");
                    // Stop if we have 16 bits per sample and data length is not divisible by two.
                    if (file_bits == 16 && file_data_length % 2 != 0)
                        Program::Error("The file uses 16 bits per sample, but data size is not a multiple of two.");
                    // Stop if the file is too short.
                    if (file.size() < file_data_length + 44)
                        Program::Error("Unexpected end of file.");
                    ptr += 4;

                    // Copy data.
                    data = std::vector<std::uint8_t>(ptr, ptr + file_data_length);

                    // Fix byte order if necessary.
                    if (bits_per_sample == bits_16)
                        for (std::size_t i = 0; i < data.size(); i += 2)
                            ByteOrder::ConvertBytes(&data[i], 2, ByteOrder::little);
                }
                break;
              case ogg:
                {
                    (void)OV_CALLBACKS_DEFAULT;
                    (void)OV_CALLBACKS_NOCLOSE;
                    (void)OV_CALLBACKS_STREAMONLY;
                    (void)OV_CALLBACKS_STREAMONLY_NOCLOSE;

                    // Construct callbacks for reading from memory buffer.
                    struct Descriptor
                    {
                        const std::uint8_t *start, *cur, *end;
                    };
                    Descriptor desc{file.begin(), file.begin(), file.end()};

                    ov_callbacks callbacks;
                    callbacks.tell_func = [](void *desc_ptr) -> long
                    {
                        Descriptor &desc = *static_cast<Descriptor *>(desc_ptr);
                        return desc.cur - desc.start;
                    };
                    callbacks.seek_func = [](void *desc_ptr, std::int64_t offset, int mode) -> int
                    {
                        Descriptor &desc = *static_cast<Descriptor *>(desc_ptr);

                        switch (mode)
                        {
                          case SEEK_SET:
                            desc.cur = desc.start + offset;
                            break;
                          case SEEK_CUR:
                            desc.cur += offset;
                            break;
                          case SEEK_END:
                            desc.cur = desc.end + offset;
                            break;
                          default:
                            return 1;
                        }

                        if (desc.cur < desc.start || desc.cur > desc.end)
                            return 1;

                        return 0;
                    };
                    callbacks.read_func = [](void *dst, std::size_t elem_size, std::size_t elem_count, void *desc_ptr) -> std::size_t
                    {
                        Descriptor &desc = *static_cast<Descriptor *>(desc_ptr);
                        if (desc.cur + elem_count * elem_size > desc.end)
                            elem_count = (desc.end - desc.cur) / elem_size;
                        std::copy(desc.cur, desc.cur + elem_count * elem_size, static_cast<std::uint8_t *>(dst));
                        desc.cur += elem_count * elem_size;
                        return elem_count;
                    };
                    callbacks.close_func = 0;

                    // Open file.
                    OggVorbis_File ogg_file;
                    switch (ov_open_callbacks(&desc, &ogg_file, 0, 0, callbacks))
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
                    FINALLY( ov_clear(&ogg_file); )

                    // Get file info.
                    vorbis_info *info = ov_info(&ogg_file, -1);

                    // Stop if the file is too big.
                    uint64_t samples = ov_pcm_total(&ogg_file, -1);
                    if (samples > std::numeric_limits<std::size_t>::max())
                        Program::Error("The file is too big.");

                    // Get channel count.
                    if (info->channels != 1 && info->channels != 2)
                        Program::Error("The file has too many channels. Only mono and stereo are supported.");
                    channel_count = Channels(info->channels);

                    // Get frequency.
                    sampling_rate = info->rate;

                    // Save the preferred amount of bits per sample.
                    bits_per_sample = preferred_bits_per_sample;

                    // Reserve the necessary amount of memory.
                    std::size_t remaining_bytes = samples * BytesPerSample();
                    data.resize(remaining_bytes);
                    char *target = reinterpret_cast<char *>(data.data());

                    // Figure out reading options.
                    int bytes_per_sample = bits_per_sample == bits_16 ? 2 : 1;
                    bool signed_samples = bits_per_sample == bits_16;

                    // Read the file.
                    int current_bitstream = -1;
                    while (remaining_bytes > 0)
                    {
                        // Try reading some bytes.
                        int bitstream;
                        long section_size = ov_read(&ogg_file, target, remaining_bytes, ByteOrder::native == ByteOrder::big, bytes_per_sample, signed_samples, &bitstream);

                        // Check for errors.
                        switch (section_size)
                        {
                          case 0:
                            Program::Error("Unexpected end of stream.");
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
                          default:
                            break;
                        }

                        // Check if the bitstream has changed.
                        if (bitstream != current_bitstream)
                        {
                            current_bitstream = bitstream;

                            // Get new information.
                            vorbis_info *local_info = ov_info(&ogg_file, bitstream);

                            // Stop if the amount of channels has changed.
                            if (local_info->channels != info->channels)
                                Program::Error("Different sections of the file contain different number of channels. This is not supported.");

                            // Stop if the frequency has changed.
                            if (local_info->rate != info->rate)
                                Program::Error("Different sections of the file use different sampling rates. This is not supported.");
                        }

                        // Adjust data pointer and the remaining byte count.
                        target += section_size;
                        remaining_bytes -= section_size;
                    }
                }
                break;
            }
        }
        catch (std::exception &e)
        {
            std::string format_name;
            switch (format)
            {
              case wav:
                format_name = "WAV";
                break;
              case ogg:
                format_name = "OGG";
                break;
            }

            Program::Error("Unable to parse `", file.name(), "` as ", format_name, ":\n", e.what());
        }
    }

}
