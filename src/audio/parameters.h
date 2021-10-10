#pragma once

#include "audio/openal.h"
#include "utils/mat.h"

namespace Audio
{
    // Global volume, defaults to 1.
    inline void Volume(float vol)
    {
        alListenerf(AL_GAIN, vol);
    }

    // Listener position.
    inline void ListenerPosition(fvec3 pos)
    {
        alListenerfv(AL_POSITION, pos.as_array());
    }
    // Listener velocity. Likely measured in units per second.
    // Defaults to 0, and can be left alone if doppler effects don't interest you.
    inline void ListenerVelocity(fvec3 vel)
    {
        alListenerfv(AL_VELOCITY, vel.as_array());
    }
    // Listener orientation, aka rotation.
    // Both vectors are relative to the listener.
    // AL uses the same coorinate system as GL: right-handed, by default X is right, Y is up, Z is towards the listener.
    inline void ListenerOrientation(fvec3 forward, fvec3 up)
    {
        fvec3 array[2] = {forward, up};
        alListenerfv(AL_ORIENTATION, array[0].as_array());
    }

    namespace Parameters
    {
        // Defaults to 1.
        inline void DopplerFactor(float n)
        {
            alDopplerFactor(n);
        }

        // Defaults to 343.3 (see internal macro SPEEDOFSOUNDMETRESPERSEC).
        // Measured in units per second. The default value assumes air as the medium, and meters as the units.
    	inline void SpeedOfSound(float n)
    	{
    	    alSpeedOfSound(n);
        }

        // This is not listed in the original AL spec and was found by looking at the code.
        // Defaults to 1 (see AL_DEFAULT_METERS_PER_UNIT).
        // It seems to affect air absorption calculations, reverb, HRTF ("head-related transfer function", aka improved headphone sound), and possibly something else.
        inline void MetersPerUnit(float n)
        {
            alListenerf(AL_METERS_PER_UNIT, n);
        }


        enum Model
        {
            // Here `gain` means the volume factor.

            // gain = 1
            // Clamping has no effect on this model.
            none,

            // According to the spec:
            //     gain = ref_dist / (ref_dist + rolloff_fac * (dist - ref_dist))
            // This can be rewritten as:
            //     gain = 1 / (1 + rolloff_fac * (dist / ref_dist - 1))
            // In simple terms, `gain` follows a hyperbola, which is scaled and translated according to the parameters.
            // You start with `gain = 1 / dist`. (Which is what you get if `ref_dist = 1` and `rolloff_fac = 1`.)
            // Then you scale it along the X axis by `ref_dist`, relative to 0. It means that for `dist = ref_dist`, gain is always 1.
            // Then you scale it by `1 / rolloff_fac` along the X axis, relative to `ref_dst`. This, in turn, means that if you
            //   keep `rolloff_fac` proportional to `ref_dist`, then changing `ref_dist` will translate the hyperbola along the X axis without scaling it.
            inverse,

            // Gain is 1 at `dist = ref_dist` and is 0 at `dist = max_dist`.
            // If `rolloff_factor != 1`, then `max_dist` is replaced with `(max_dist - ref_dist) / rolloff_fac + ref_dist`.
            // Even if clamping is disabled, the resulting gain can't be less than 0.
            // Note that even though the original AL spec seems to imply that if `rolloff_fac > 1`, then the gain
            //   should become negative for some distances, OpenAL-soft does the right thing and clamps it at 0.
            linear,

            // gain = pow(dist / ref_dist, -rolloff_fac)
            exponential,
        };

        // If clamping is enabled, the distance is clamped between `ref_dist` and `max_dist`. Disabling clamping is not recommended.
        // In any case, according to the spec, if the parameters make gain calculations impossible, the gain falls back to 1.
        inline void DistanceModel(Model model, bool clamped = true)
        {
            ALenum model_enum = AL_NONE;
            switch (model)
            {
              case none:
                // Keep AL_NONE.
                break;
              case inverse:
                model_enum = clamped ? AL_INVERSE_DISTANCE_CLAMPED : AL_INVERSE_DISTANCE;
                break;
              case linear:
                model_enum = clamped ? AL_LINEAR_DISTANCE_CLAMPED : AL_LINEAR_DISTANCE;
                break;
              case exponential:
                model_enum = clamped ? AL_EXPONENT_DISTANCE_CLAMPED : AL_EXPONENT_DISTANCE;
                break;
            }
            alDistanceModel(model_enum);
        }
    }
}
