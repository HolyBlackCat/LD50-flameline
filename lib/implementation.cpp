#include "errors.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_TGA
#define STBI_ASSERT(expr) DebugAssert("Somewhere in STB Image.", expr)
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_ASSERT(expr) DebugAssert("Somewhere in STB Image Write.", expr)
#include <stb_image_write.h>
