#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <string>

#include "audio/buffer.h"
#include "audio/sound.h"
#include "meta/misc.h"
#include "meta/string_template_params.h"
#include "program/errors.h"

namespace Audio
{
    namespace impl
    {
        struct AutoLoadedBuffer
        {
            Buffer buffer;
            // Those may override the parameters specified when calling `LoadMentionedFiles()`.
            std::optional<Channels> channels_override;
            std::optional<Format> format_override;
        };

        // We rely on `std::map` never invalidating the references.
        using AutoLoadedBuffersMap = std::map<std::string, AutoLoadedBuffer, std::less<>>;

        AutoLoadedBuffersMap &GetAutoLoadedBuffers()
        {
            static AutoLoadedBuffersMap ret;
            return ret;
        }

        template <typename T> concept ChannelsOrNullptr = Meta::same_as_any_of<T, Channels, std::nullptr_t>;
        template <typename T> concept FormatOrNullptr = Meta::same_as_any_of<T, Format, std::nullptr_t>;

        template <ChannelsOrNullptr auto ChannelCount, FormatOrNullptr auto FileFormat, Meta::ConstString Name>
        struct RegisterAutoLoadedBuffer
        {
            [[maybe_unused]] inline static const Buffer &ref = []() -> Buffer &
            {
                auto it = GetAutoLoadedBuffers().find(Name.str);
                ASSERT(it == GetAutoLoadedBuffers().end(), "Attempt to register a duplicate auto-loaded sound file. This shouldn't be possible.");
                AutoLoadedBuffer &data = GetAutoLoadedBuffers().try_emplace(it, Name.str)->second;
                if constexpr (!std::is_null_pointer_v<decltype(ChannelCount)>)
                    data.channels_override = ChannelCount;
                if constexpr (!std::is_null_pointer_v<decltype(FileFormat)>)
                    data.format_override = FileFormat;
                return data.buffer; // We rely on `std::map` never invalidating the references.
            }();
        };
    }

    // Returns a reference to a buffer, loaded from the filename passed as the parameter.
    // The load doesn't happen at the call point, and is done by `LoadMentionedFiles()`, which magically knows all files that it needs to load in this manner.
    template <impl::ChannelsOrNullptr auto ChannelCount = nullptr, impl::FormatOrNullptr auto FileFormat = nullptr, Meta::ConstString Name>
    [[nodiscard]] const Buffer &File(Meta::ConstStringParam<Name>)
    {
        return impl::RegisterAutoLoadedBuffer<ChannelCount, FileFormat, Name>::ref;
    }

    // Loads (or reloads) all files mentioned in all known `Audio::File(...)` calls.
    // The number of channels and the file format can be overridden by the `File()` calls.
    // `process_filename` is a function that processes filenames before use. You can use the default function returned by `LoadFromPrefix()`.
    // The signatures is `std::string (const std::string &name, std::optional<Channels> channels, Format format)`, it processes the filenames before loading them.
    inline void LoadMentionedFiles(auto &&process_filename, std::optional<Channels> channels, Format format)
    {
        for (auto &[name, data] : impl::GetAutoLoadedBuffers())
        {
            std::optional<Channels> file_channels = data.channels_override ? data.channels_override : channels;
            Format file_format = data.format_override.value_or(format);
            data.buffer = Audio::Sound(file_format, file_channels, process_filename(name, file_channels, file_format));
        }
    }

    // A default callback for `LoadMentionedFiles()`.
    // Adds the prefix to filenames, and automatically adds extensions matching the file type.
    inline auto LoadFromPrefixWithExt(std::string prefix)
    {
        return [prefix](const std::string &name, std::optional<Channels> channels, Format format) -> std::string
        {
            (void)channels;
            const char *ext = "";
            switch (format)
            {
                case wav: ext = ".wav"; break;
                case ogg: ext = ".ogg"; break;
            }
            return prefix + name + ext;
        };
    }
}
