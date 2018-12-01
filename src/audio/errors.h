#pragma once

#include <AL/alc.h>

#include "audio/context.h"
#include "program/errors.h"

namespace Audio
{
    inline void CheckErrors()
    {
        switch (alcGetError(Context::Get().DeviceHandle()))
        {
            case 0:                   return;
            case ALC_INVALID_DEVICE:  Program::Error("OpenAL error: Invalid device.");
            case ALC_INVALID_CONTEXT: Program::Error("OpenAL error: Invalid context.");
            case ALC_INVALID_ENUM:    Program::Error("OpenAL error: Invalid enum.");
            case ALC_INVALID_VALUE:   Program::Error("OpenAL error: Invalid value.");
            case ALC_OUT_OF_MEMORY:   Program::Error("OpenAL error: Out of memory.");
            default:                  Program::Error("OpenAL error: Unknown error.");
        }
    }
}
