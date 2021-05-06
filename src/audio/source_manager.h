#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "audio/buffer.h"
#include "audio/source.h"
#include "program/errors.h"

namespace Audio
{
    // Keeps a list of `std::shared_ptr`s to sources.
    // Automatically releases them when they stop playing.
    class SourceManager
    {
        std::vector<std::shared_ptr<Source>> sources;

      public:
        SourceManager() {}

        // Adds a new source to the manager.
        // It should be `play()`ed immediately, otherwise it will be removed at the next `Tick()`.
        void Add(std::shared_ptr<Source> source)
        {
            ASSERT(std::find(sources.begin(), sources.end(), source) == sources.end(), "Adding a duplicate source to `Audio::SourceManager`.");
            sources.push_back(std::move(source));
        }
        [[nodiscard]] std::shared_ptr<Source> Add(const Buffer &buffer)
        {
            return sources.emplace_back(std::make_shared<Source>(buffer));
        }

        // Releases sources that aren't playing (i.e. are stopped, paused, or not played yet).
        void Tick()
        {
            std::erase_if(sources, [](const std::shared_ptr<Source> &ptr){return !ptr->IsPlaying();});
        }

        [[nodiscard]] std::size_t ActiveSources() const
        {
            return sources.size();
        }
    };
}
