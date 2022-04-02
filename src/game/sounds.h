#pragma once

#include "game/main.h"

#define SOUND_LIST(X) \
    X( jump    , 0.3 ) \
    X( landing , 0.3 ) \


namespace Sounds
{
    #define MAKE_SOUND(name, randpitch) \
        std::shared_ptr<Audio::Source> name(std::optional<ivec2> pos, float volume = 1, float pitch = 1) \
        { \
            auto ret = audio_controller.Add(Audio::File<#name>()); \
            if (pos) \
                ret->pos(*pos); \
            else \
                ret->relative(); \
            ret->volume(volume).pitch(pow(2, pitch - (ra.f.abs() <= randpitch))).play(); \
            return ret; \
        } \
        std::shared_ptr<Audio::Source> name(float volume = 1, float pitch = 1) \
        { \
            return name({}, volume, pitch); \
        }
    SOUND_LIST(MAKE_SOUND)
    #undef MAKE_SOUND
}
