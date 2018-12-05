#pragma once

#include <limits>
#include <memory>
#include <unordered_set>

#include <AL/al.h>

#include "audio/buffer.h"
#include "utils/mat.h"

namespace Audio
{
    class Source
    {
        inline static float
            default_ref_dist    = 1,
            default_rolloff_fac = 1,
            default_max_dist    = std::numeric_limits<float>::infinity();

        struct Object
        {
            ALuint handle = 0;

            Object()
            {
                // We don't throw the handle is null. Instead, we make sure that any operation on a null handle has no effect.
                alGenSources(1, &handle);

                if (handle)
                {
                    alSourcef(handle, AL_REFERENCE_DISTANCE, default_ref_dist);
                    alSourcef(handle, AL_ROLLOFF_FACTOR,     default_rolloff_fac);
                    alSourcef(handle, AL_MAX_DISTANCE,       default_max_dist);
                }
            }

            Object(const Object &) = delete;
            Object &operator=(const Object &) = delete;

            ~Object()
            {
                if (handle)
                    alDeleteSources(1, &handle);
            }
        };

        inline static std::unordered_set<std::shared_ptr<Object>> destroyed_temporary_sources;

        struct Data
        {
            std::shared_ptr<Object> object;
            bool is_temporary = 0;
        };
        Data data;

        using ref = Source &&;

      public:
        Source(decltype(nullptr)) {}

        Source(const Audio::Buffer &buffer)
        {
            // Create the source.
            data.object = std::make_shared<Object>();
            if (data.object->handle)
                alSourcei(data.object->handle, AL_BUFFER, buffer.Handle());
            else
                data.object = {};

            data.is_temporary = 0;
        }

        Source(Source &&other) noexcept : data(std::exchange(other.data, {})) {}
        Source &operator=(Source other) noexcept
        {
            std::swap(data, other.data);
            return *this;
        }

        ~Source()
        {
            if (bool(data.object) && data.is_temporary)
            {
                int is_looping;
                alGetSourcei(data.object->handle, AL_LOOPING, &is_looping);
                if (is_looping)
                    return;
                play();
                destroyed_temporary_sources.insert(data.object);
            }
        }

        explicit operator bool() const
        {
            return bool(data.object);
        }

        // Cleans up destroyed temporary sources that finished playing.
        // Normally you don't need to call it manually, since it's called from `Context::Tick()`.
        // If it is not called, you'll gradually leak memory as the sources finish playing.
        static void Cleanup()
        {
            auto it = destroyed_temporary_sources.begin();

            while (it != destroyed_temporary_sources.end())
            {
                int state;
                alGetSourcei((*it)->handle, AL_SOURCE_STATE, &state);
                if (state != AL_PLAYING)
                    it = destroyed_temporary_sources.erase(it);
                else
                    it++;
            }
        }

        /* The volume curve is a hyperbola, clamped at 1 (if `distance` < `ref`).
         *             /                   1                \
         * volume = min| 1 , ------------------------------ |
         *             \     1 + fac * (distance * ref - 1) /
         * `distance` is clamped at `max`.
         */
        static void DefaultMaxDistance(float d)
        {
            default_max_dist = d;
        }
        static void DefaultRefDistance(float d)
        {
            default_ref_dist = d;
        }
        static void DefaultRolloffFactor(float f)
        {
            default_rolloff_fac = f;
        }


        ref max_distance(float d)
        {
            if (data.object)
                alSourcef(data.object->handle, AL_MAX_DISTANCE, d);
            return (ref)*this;
        }
        ref ref_distance(float d)
        {
            if (data.object)
                alSourcef(data.object->handle, AL_REFERENCE_DISTANCE, d);
            return (ref)*this;
        }
        ref rolloff_factor(float f)
        {
            if (data.object)
                alSourcef(data.object->handle, AL_ROLLOFF_FACTOR, f);
            return (ref)*this;
        }

        ref temporary() // Doesn't work for looped sounds. Plays the sound after the data.object is destroyed.
        {
            if (data.object)
                data.is_temporary = 1;
            return (ref)*this;
        }

        ref volume(float v)
        {
            if (data.object)
                alSourcef(data.object->handle, AL_GAIN, v);
            return (ref)*this;
        }
        ref pitch(float p)
        {
            if (data.object)
                alSourcef(data.object->handle, AL_PITCH, p);
            return (ref)*this;
        }
        ref loop(float l)
        {
            if (data.object)
                alSourcei(data.object->handle, AL_LOOPING, l);
            return (ref)*this;
        }

        ref play()
        {
            if (data.object)
                alSourcePlay(data.object->handle);
            return (ref)*this;
        }
        ref pause()
        {
            if (data.object)
                alSourcePause(data.object->handle);
            return (ref)*this;
        }
        ref stop()
        {
            if (data.object)
                alSourceStop(data.object->handle);
            return (ref)*this;
        }
        ref rewind()
        {
            if (data.object)
                alSourceRewind(data.object->handle);
            return (ref)*this;
        }

        // All functions below do not work for stereo sources.

        ref pos(fvec3 p)
        {
            if (data.object)
                alSourcefv(data.object->handle, AL_POSITION, p.as_array());
            return (ref)*this;
        }
        ref vel(fvec3 v)
        {
            if (data.object)
                alSourcefv(data.object->handle, AL_VELOCITY, v.as_array());
            return (ref)*this;
        }
        ref relative(bool r = 1)
        {
            if (data.object)
                alSourcei(data.object->handle, AL_SOURCE_RELATIVE, r);
            return (ref)*this;
        }

        ref pos(fvec2 p)
        {
            pos(p.to_vec3());
            return (ref)*this;
        }
        ref vel(fvec2 p)
        {
            vel(p.to_vec3());
            return (ref)*this;
        }
    };
}
