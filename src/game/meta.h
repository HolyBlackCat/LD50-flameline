#pragma once

#include "game/adaptive_viewport.h"
#include "game/render.h"
#include "graphics/texture_atlas.h"
#include "input/mouse.h"
#include "interface/window.h"
#include "utils/mat.h"
#include "utils/metronome.h"
#include "utils/random.h"

extern const ivec2 screen_size;
extern Interface::Window win;
extern Random<> random; // <> can't be omitted, see `meta.cpp` for details.
extern Render render;
extern AdaptiveViewport viewport;
extern Metronome metronome;
extern Input::Mouse mouse;
const Graphics::TextureAtlas &Atlas();
