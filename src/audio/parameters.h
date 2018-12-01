#pragma once

#include <AL/al.h>

#include "utils/mat.h"

namespace Audio
{
    inline void Volume(float vol)
    {
        alListenerf(AL_GAIN, vol);
    }
    inline void Pitch(float pitch)
    {
        alListenerf(AL_PITCH, pitch);
    }

    namespace Listener
    {
        inline void Position(fvec3 pos)
        {
            alListenerfv(AL_POSITION, pos.as_array());
        }
        inline void Velocity(fvec3 vel)
        {
            alListenerfv(AL_VELOCITY, vel.as_array());
        }
        inline void Orientation(fvec3 forward, fvec3 up)
        {
            fvec3 array[2] = {forward, up};
            alListenerfv(AL_ORIENTATION, array[0].as_array());
        }
    }

    inline void DopplerFactor(float n)
    {
        alDopplerFactor(n);
    }
	inline void SpeedOfSound(float n)
	{
	    alSpeedOfSound(n);
    }
}
