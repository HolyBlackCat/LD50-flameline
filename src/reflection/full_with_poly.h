#pragma once

#include "reflection/full.h"
#include "reflection/poly_storage_support.h"

// This file is one of the few public reflection headers.
//
// .- full_with_poly.h -----------------------.     .- short_macros.h ----------.
// |                                          |     |                           |
// |  .- full.h ---------------------------.  |     | Alternative short macros. |
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
