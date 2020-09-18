#pragma once

#include <limits>
#include <memory>
#include <unordered_set>

#include "audio/buffer.h"
#include "audio/openal.h"
#include "program/errors.h"
#include "utils/mat.h"

namespace Audio
{
    enum SourceState {playing, paused, stopped};

    class Source
    {
        inline static float
            default_rolloff_fac = 1,
            default_ref_dist    = 1,
            default_max_dist    = std::numeric_limits<float>::infinity();

        struct Data
        {
            ALuint handle = 0;
        };
        Data data;

        using ref = Source &;

      public:
        // Create a null source.
        Source() {}

        Source(const Audio::Buffer &buffer)
        {
            ASSERT(buffer, "Attempt to use a null audio buffer.");

            // We don't throw if the handle is null. Instead, we make sure that any operation on a null handle has no effect.
            alGenSources(1, &data.handle);

            if (data.handle)
            {
                alSourcei(data.handle, AL_BUFFER, buffer.Handle());

                alSourcef(data.handle, AL_REFERENCE_DISTANCE, default_ref_dist);
                alSourcef(data.handle, AL_ROLLOFF_FACTOR,     default_rolloff_fac);
                alSourcef(data.handle, AL_MAX_DISTANCE,       default_max_dist);
            }
        }

        Source(Source &&other) noexcept : data(std::exchange(other.data, {})) {}
        Source &operator=(Source other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        ~Source()
        {
            if (data.handle)
                alDeleteSources(1, &data.handle);
        }

        // Returns true if the source is not null.
        // Note that if a non-null source can't be constructed, you'll silently get a null source instead.
        // All operations on a null source should be no-ops.
        [[nodiscard]] explicit operator bool() const
        {
            return bool(data.handle);
        }

        // Returns the source handle.
        [[nodiscard]] ALuint Handle() const
        {
            return data.handle;
        }

        [[nodiscard]] SourceState GetState() const
        {
            if (!data.handle)
                return stopped;
            int state;
            alGetSourcei(data.handle, AL_SOURCE_STATE, &state);
            switch (state)
            {
              case AL_INITIAL:
              case AL_STOPPED:
                return stopped;
                break;
              case AL_PLAYING:
                return playing;
                break;
              case AL_PAUSED:
                return paused;
                break;
            }
            return stopped;
        }

        [[nodiscard]] bool IsLooping() const
        {
            if (!data.handle)
                return false;
            int ret;
            alGetSourcei(data.handle, AL_LOOPING, &ret);
            return bool(ret);
        }


        // Sound model parameters.
        // See comments for `Audio::Parameters::Model` in `parameters.h` for the meaning of those settings.

        // Defaults to 1. Increase to make the sound lose volume with distance faster.
        static void DefaultRolloffFactor(float f)
        {
            default_rolloff_fac = f;
        }
        static void DefaultRefDistance(float d)
        {
            default_ref_dist = d;
        }
        static void DefaultMaxDistance(float d)
        {
            default_max_dist = d;
        }

        Source &rolloff_factor(float f)
        {
            if (data.handle)
                alSourcef(data.handle, AL_ROLLOFF_FACTOR, f);
            return *this;
        }
        Source &max_distance(float d)
        {
            if (data.handle)
                alSourcef(data.handle, AL_MAX_DISTANCE, d);
            return *this;
        }
        Source &ref_distance(float d)
        {
            if (data.handle)
                alSourcef(data.handle, AL_REFERENCE_DISTANCE, d);
            return *this;
        }


        // Common parameters.

        Source &volume(float v)
        {
            if (data.handle)
                alSourcef(data.handle, AL_GAIN, v);
            return *this;
        }
        Source &pitch(float p)
        {
            if (data.handle)
                alSourcef(data.handle, AL_PITCH, p);
            return *this;
        }
        Source &loop(bool l = true)
        {
            if (data.handle)
                alSourcei(data.handle, AL_LOOPING, l);
            return *this;
        }


        // State control.

        Source &play()
        {
            if (data.handle)
                alSourcePlay(data.handle);
            return *this;
        }
        Source &pause()
        {
            if (data.handle)
                alSourcePause(data.handle);
            return *this;
        }
        Source &stop()
        {
            if (data.handle)
                alSourceStop(data.handle);
            return *this;
        }
        Source &rewind()
        {
            if (data.handle)
                alSourceRewind(data.handle);
            return *this;
        }


        // 3D audio support (makes sense for mono sources only).

        Source &pos(fvec3 p)
        {
            if (data.handle)
                alSourcefv(data.handle, AL_POSITION, p.as_array());
            return *this;
        }
        Source &vel(fvec3 v)
        {
            if (data.handle)
                alSourcefv(data.handle, AL_VELOCITY, v.as_array());
            return *this;
        }
        Source &relative(bool r = true)
        {
            if (data.handle)
                alSourcei(data.handle, AL_SOURCE_RELATIVE, r);
            return *this;
        }

        Source &pos(fvec2 p) {return pos(p.to_vec3());}
        Source &vel(fvec2 p) {return vel(p.to_vec3());}
    };
}
