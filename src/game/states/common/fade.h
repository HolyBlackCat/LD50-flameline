#pragma once

#include <functional>

#include "game/state.h"
#include "utils/dynamic_storage.h"
#include "utils/mat.h"

namespace States::Details::Common
{
    class Fade
    {
        fvec3 color;
        float alpha = 1;
        bool have_next_state = 0;
        DynStorage<State> next_state;
      public:
        Fade(fvec3 color) : color(color) {}
        void Tick();
        void Render() const;

        void SetNextState(DynStorage<State> next_state); // Pass null state to exit from the program.
        bool ChangingState() const;
    };
}
