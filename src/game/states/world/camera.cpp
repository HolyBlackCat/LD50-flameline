#include "program/parachute.h"

#include <utility>
#include <vector>

#include "game/states/world/camera.h"
#include "game/meta.h"
#include "game/controls.h"

namespace States::Details::World
{
    void Camera::Tick()
    {
        SetPos(controls.mouse.pos());
    }
}
