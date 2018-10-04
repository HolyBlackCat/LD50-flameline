#include "program/parachute.h"

#include <utility>
#include <vector>

#include "game/states/world/camera.h"
#include "game/meta.h"

namespace States::Details::World
{
    void Camera::Tick()
    {
        SetPos(mouse.pos());
    }
}
