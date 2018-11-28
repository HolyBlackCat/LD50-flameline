#pragma once

#include <cstddef>

#include "utils/range_set.h"

namespace Unicode
{
    using std::uint32_t;

    using CharSet = RangeSet<uint32_t>;

    inline constexpr uint32_t default_char = 0xfffd;
}
