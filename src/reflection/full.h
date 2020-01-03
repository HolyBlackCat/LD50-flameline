#pragma once

#include "reflection/full_without_poly.h"
#include "reflection/poly_storage_support.h"

// This file is one of the few public reflection headers.
//
// .- full.h ---------------------------------.     .- short_macros.h ----------.
// |                                          |     |                           |
// |  .- full_without_poly.h --------------.  |     | Alternative short macros. |
// |  |                                    |  |     '---------------------------'
// |  | Serialization and deserialization. |  |
// |  |                                    |  |
// |  |  .- structs.h ----------------.    |  |
// |  |  |                            |    |  |
// |  |  | Class metadata inspection. |    |  |
// |  |  '----------------------------'    |  |
// |  '------------------------------------'  |
// |                                          |
// | Polymorphic class support.               |
// '------------------------------------------'
