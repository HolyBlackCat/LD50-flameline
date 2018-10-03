#pragma once

#include "game/adaptive_viewport.h"
#include "game/render.h"
#include "game/texture_atlas.h"
#include "input/mouse.h"
#include "interface/window.h"
#include "utils/config.h"
#include "utils/mat.h"
#include "utils/metronome.h"
#include "utils/random.h"

extern const ivec2 screen_size;
extern Interface::Window win;
extern Random<> random; // <> can't be omitted, see `meta.cpp` for details.
extern Render render;
const TextureAtlas &Atlas();
extern AdaptiveViewport viewport;
extern Metronome metronome;
extern Input::Mouse mouse;
