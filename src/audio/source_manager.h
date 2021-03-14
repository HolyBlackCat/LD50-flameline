#pragma once

#include <cstddef>
#include <memory>
#include <vector>

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

        void AddSource(std::shared_ptr<Source> source)
        {
            ASSERT(std::find(sources.begin(), sources.end(), source) == sources.end(), "Adding a duplicate source to `Audio::SourceManager`.");
            sources.push_back(std::move(source));
        }

        // Releases sources that aren't playing.
        void Tick()
        {
            std::erase_if(sources, [](const std::shared_ptr<Source> &ptr){return ptr->GetState() == stopped;});
        }

        [[nodiscard]] std::size_t ActiveSources() const
        {
            return sources.size();
        }
    };
}
